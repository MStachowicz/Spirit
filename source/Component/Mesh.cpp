#include "Mesh.hpp"

// UTILITY
#include "Logger.hpp"

// ASSIMP
#include "assimp/Importer.hpp"
#include "assimp/postprocess.h"
#include "assimp/scene.h"

namespace Data
{
    Mesh::Mesh(aiMesh& pAssimpMesh) noexcept
        : mPositions{}
        , mNormals{}
        , mTextureCoordinates{}
        , mIndices{}
        , mMaterial{}
        , mAABB{}
        , mTriangles{}
        , mGLData{}
    {
        ASSERT(pAssimpMesh.mNumUVComponents[0] == 2, "Only 2-component UVs are supported");
        ASSERT(pAssimpMesh.HasNormals(), "Mesh has to have normals");

         // Process positions, normals and texture coordinates
        for (unsigned int i = 0; i < pAssimpMesh.mNumVertices; i++)
        {
            mPositions.emplace_back(glm::vec3{pAssimpMesh.mVertices[i].x, pAssimpMesh.mVertices[i].y, pAssimpMesh.mVertices[i].z});
            mAABB.unite(mPositions.back());
            mNormals.emplace_back(glm::vec3{pAssimpMesh.mNormals[i].x, pAssimpMesh.mNormals[i].y, pAssimpMesh.mNormals[i].z});
            // A vertex can contain up to 8 texture coordinates. We make the assumption that we won't use models where a vertex
            // can have multiple texture coordinates so we always take the first set (0).
            mTextureCoordinates.emplace_back(glm::vec2{pAssimpMesh.mTextureCoords[0][i].x, pAssimpMesh.mTextureCoords[0][i].y});
        }

        // Process indices
        for (unsigned int i = 0; i < pAssimpMesh.mNumFaces; i++)
        {
            aiFace face = pAssimpMesh.mFaces[i];
            for (unsigned int j = 0; j < face.mNumIndices; j++)
            {
                mIndices.push_back(face.mIndices[j]);
            }
        }

        // Iterate the mPositions, (in mIndices order if they exist) to extract the object space triangles of the mesh.
        if (!mIndices.empty())
        {
            for (size_t i = 0; i < mIndices.size(); i+= 3)
                mTriangles.emplace_back<Geometry::Triangle>({mPositions[mIndices[i]], mPositions[mIndices[i + 1]], mPositions[mIndices[i + 2]]});
        }
        else
        {
            for (size_t i = 0; i < mPositions.size(); i+= 3)
                mTriangles.emplace_back<Geometry::Triangle>({mPositions[i], mPositions[i + 1], mPositions[i + 2]});
        }

        // After all the vertex data is initialised, call the OpenGL constructor to push the data to the GPU
        // ADD A MOVE ASSIGNMENT FOR THE MESH CLASS..
        mGLData = std::move(OpenGL::Mesh(*this));
    }

    Mesh::Mesh(aiMesh& pAssimpMesh, const aiScene& pAssimpScene, TextureManager& pTextureManager) noexcept
        : mPositions{}
        , mNormals{}
        , mTextureCoordinates{}
        , mIndices{}
        , mMaterial{}
        , mAABB{}
        , mTriangles{}
        , mGLData{}
    {
        ASSERT(pAssimpMesh.mNumUVComponents[0] == 2, "Only 2-component UVs are supported");
        ASSERT(pAssimpMesh.HasNormals(), "Mesh has to have normals");

        // Process positions, normals and texture coordinates
        for (unsigned int i = 0; i < pAssimpMesh.mNumVertices; i++)
        {
            mPositions.emplace_back(glm::vec3{pAssimpMesh.mVertices[i].x, pAssimpMesh.mVertices[i].y, pAssimpMesh.mVertices[i].z});
            mAABB.unite(mPositions.back());
            mNormals.emplace_back(glm::vec3{pAssimpMesh.mNormals[i].x, pAssimpMesh.mNormals[i].y, pAssimpMesh.mNormals[i].z});
            // A vertex can contain up to 8 texture coordinates. We make the assumption that we won't use models where a vertex
            // can have multiple texture coordinates so we always take the first set (0).
            mTextureCoordinates.emplace_back(glm::vec2{pAssimpMesh.mTextureCoords[0][i].x, pAssimpMesh.mTextureCoords[0][i].y});
        }

        // Process indices
        for (unsigned int i = 0; i < pAssimpMesh.mNumFaces; i++)
        {
            aiFace face = pAssimpMesh.mFaces[i];
            for (unsigned int j = 0; j < face.mNumIndices; j++)
            {
                mIndices.push_back(face.mIndices[j]);
            }
        }

        // Iterate the mPositions, (in mIndices order if they exist) to extract the object space triangles of the mesh.
        if (!mIndices.empty())
        {
            for (size_t i = 0; i < mIndices.size(); i+= 3)
                mTriangles.emplace_back<Geometry::Triangle>({mPositions[mIndices[i]], mPositions[mIndices[i + 1]], mPositions[mIndices[i + 2]]});
        }
        else
        {
            for (size_t i = 0; i < mPositions.size(); i+= 3)
                mTriangles.emplace_back<Geometry::Triangle>({mPositions[i], mPositions[i + 1], mPositions[i + 2]});
        }

        if (aiMaterial* material = pAssimpScene.mMaterials[pAssimpMesh.mMaterialIndex])
        {
            auto processTextureType = [&material, &pTextureManager](const aiTextureType& pTextureType)
            {
                for (unsigned int i = 0; i < material->GetTextureCount(pTextureType); i++)
                {
                    aiString fileName;
                    material->GetTexture(pTextureType, i, &fileName);
                    const std::filesystem::path textureFilePath = fileName.C_Str();
                    ASSERT(false, "Need to set textureFilePath to full path for textureSystem loader");

                    return std::make_optional(pTextureManager.get_or_create([&textureFilePath](const Texture& pTexture)
                        { return pTexture.m_image_ref->m_filepath == textureFilePath; }, textureFilePath));
                }

                std::nullopt;
            };

            //mMaterial.mDiffuseTexture = processTextureType(aiTextureType_DIFFUSE);
            //mMaterial.mSpecularMap    = processTextureType(aiTextureType_SPECULAR);
            //mMaterial.mHeightMap      = processTextureType(aiTextureType_HEIGHT);
            //mMaterial.mAmbientMap     = processTextureType(aiTextureType_AMBIENT);
        }

        // After all the vertex data is initialised, call the OpenGL constructor to push the data to the GPU
        mGLData = std::move(OpenGL::Mesh(*this));
    }

    CompositeMesh::CompositeMesh(aiNode& pAssimpNode, const aiScene& pAssimpScene, TextureManager& pTextureManager) noexcept
        : mMeshes{}
        , mChildMeshes{}
        , mAABB{}
    {
        // Process all the node's meshes (if any)
        for (unsigned int i = 0; i < pAssimpNode.mNumMeshes; i++)
        {
            if (aiMesh* mesh = pAssimpScene.mMeshes[pAssimpNode.mMeshes[i]])
            {
                mMeshes.emplace_back(Mesh{*mesh, pAssimpScene, pTextureManager});
                mAABB.unite(mMeshes.back().mAABB);
            }
        }
        // Then do the same for each of its children
        for (unsigned int i = 0; i < pAssimpNode.mNumChildren; i++)
        {
            mChildMeshes.emplace_back(CompositeMesh{*pAssimpNode.mChildren[i], pAssimpScene, pTextureManager});
            mAABB.unite(mChildMeshes.back().mAABB);
        }
    }

    Model::Model(const std::filesystem::path& pFilePath, TextureManager& pTextureManager) noexcept
        : mFilePath{pFilePath}
        , mCompositeMesh{}
    {
        // Create an instance of the Importer class then reads file storings the scene data in the "scene" variable.
        Assimp::Importer importer;
        const aiScene* scene = importer.ReadFile(pFilePath.string(), aiProcess_Triangulate);// | aiProcess_FlipUVs);

        // Check for errors
        if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
        {
            LOG_ERROR("Model load error: {}", std::string(importer.GetErrorString()));
            ASSERT(false, "Failed to load model using ASSIMP");
            return;
        }

        mCompositeMesh = CompositeMesh(*scene->mRootNode, *scene, pTextureManager);
    }
}

namespace Component
{
    Mesh::Mesh(const ModelRef& pModel)
    : mModel{pModel}
    {}
}