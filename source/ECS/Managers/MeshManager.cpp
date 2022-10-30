#include "MeshManager.hpp"

#include "Logger.hpp"
#include "TextureManager.hpp"

#include "assimp/Importer.hpp"
#include "assimp/postprocess.h"
#include "assimp/scene.h"

#include <File.hpp>

namespace Manager
{
    MeshID MeshManager::getMeshID(const std::string& pMeshName) const
    {
        const auto it = mMeshNames.find(pMeshName);
        ZEPHYR_ASSERT(it != mMeshNames.end(), "Could not find mesh '{}' in Mesh data store.", pMeshName);
        return mMeshes[it->second].mID;
    }

    void MeshManager::setIDRecursively(Data::Mesh& pMesh, const bool& pRootMesh)
    {
        pMesh.mID.Set(mMeshes.size() - 1);

        for (auto& childMesh : pMesh.mChildMeshes)
            setIDRecursively(childMesh, false);
    }

    void MeshManager::addMesh(Data::Mesh& pMesh)
    {
        ZEPHYR_ASSERT(mMeshNames.find(pMesh.mName) == mMeshNames.end(), "addMesh should only be called with unique mesh name");

        mMeshes.push_back(pMesh);
        Data::Mesh& newMesh = mMeshes.back();
        setIDRecursively(newMesh, true);
        mMeshNames.emplace(std::make_pair(newMesh.mName, newMesh.mID.Get()));

        ZEPHYR_ASSERT(isMeshValid(newMesh), "Adding invalid mesh");
    }

    MeshID MeshManager::loadModel(const std::filesystem::path& pFilePath)
    {
        Assimp::Importer importer;
            const aiScene* scene = importer.ReadFile(pFilePath.string(), aiProcess_Triangulate);

        if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
        {
            LOG_CRITICAL("ASSIMP error loading model: \n{}", importer.GetErrorString());
            ZEPHYR_ASSERT(false, "Failed to load model using ASSIMP");
        }

        Data::Mesh rootMesh;
            rootMesh.mName     = pFilePath.stem().string();
            rootMesh.mFilePath = pFilePath.parent_path().string();
        processNode(rootMesh, scene->mRootNode, scene);

        addMesh(rootMesh); // Only addMesh on the root, all children are contained inside the root node in mMeshes
        return rootMesh.mID;
    }

    // Recursively travel all the aiNodes and extract the per-vertex data into a Zephyr mesh object
    void MeshManager::processNode(Data::Mesh& pParentMesh, aiNode* pNode, const aiScene* pScene)
    {
        for (unsigned int i = 0; i < pNode->mNumMeshes; i++)
        {
                aiMesh* mesh = pScene->mMeshes[pNode->mMeshes[i]];
            processData(pParentMesh, mesh, pScene);
        }

        for (unsigned int i = 0; i < pNode->mNumChildren; i++)
        {
            Data::Mesh childMesh;
                childMesh.mName     = pParentMesh.mName + "-child-" + std::to_string(i);
            childMesh.mFilePath = pParentMesh.mFilePath;
            processNode(childMesh, pNode->mChildren[i], pScene);
            pParentMesh.mChildMeshes.push_back(childMesh);
        }
    }

    void MeshManager::processData(Data::Mesh& pMesh, const aiMesh* pAssimpMesh, const aiScene* pAssimpScene)
{
    for (unsigned int i = 0; i < pAssimpMesh->mNumVertices; i++)
    {
        pMesh.mVertices.push_back(pAssimpMesh->mVertices[i].x);
        pMesh.mVertices.push_back(pAssimpMesh->mVertices[i].y);
        pMesh.mVertices.push_back(pAssimpMesh->mVertices[i].z);

        if (pAssimpMesh->HasNormals())
        {
            pMesh.mNormals.push_back(pAssimpMesh->mNormals[i].x);
            pMesh.mNormals.push_back(pAssimpMesh->mNormals[i].y);
            pMesh.mNormals.push_back(pAssimpMesh->mNormals[i].z);
        }
        if (pAssimpMesh->mTextureCoords[0]) // does the mesh contain texture coordinates?
        {
            // A vertex can contain up to 8 different texture coordinates. We thus make the
            // assumption that we won't use models where a vertex can have multiple
            // texture coordinates so we always take the first set (0).

            // Texture coords
            ZEPHYR_ASSERT(pAssimpMesh->mNumUVComponents[0] == 2, "Only 2-component UVs are supported");

            pMesh.mTextureCoordinates.push_back(pAssimpMesh->mTextureCoords[0][i].x);
            pMesh.mTextureCoordinates.push_back(pAssimpMesh->mTextureCoords[0][i].y);
            //// Tangent
            // pMesh.mTangents.push_back(pAssimpMesh->mTangents[i].x);
            // pMesh.mTangents.push_back(pAssimpMesh->mTangents[i].x);
            // pMesh.mTangents.push_back(pAssimpMesh->mTangents[i].x);
            //// Bitangent
            // pMesh.mBitangents.push_back(pAssimpMesh->mBitangents[i].x);
            // pMesh.mBitangents.push_back(pAssimpMesh->mBitangents[i].x);
            // pMesh.mBitangents.push_back(pAssimpMesh->mBitangents[i].x);
        }
    }

    // now wak through each of the mesh's faces and retrieve the corresponding vertex indices
    for (unsigned int i = 0; i < pAssimpMesh->mNumFaces; i++)
    {
        aiFace face = pAssimpMesh->mFaces[i];
        for (unsigned int j = 0; j < face.mNumIndices; j++)
            pMesh.mIndices.push_back(face.mIndices[j]);
    }

    { // Load the textures for the mesh
            aiMaterial* material = pAssimpScene->mMaterials[pAssimpMesh->mMaterialIndex];

            processTextures(pMesh, material, Data::Texture::Purpose::Diffuse);
            processTextures(pMesh, material, Data::Texture::Purpose::Specular);
            processTextures(pMesh, material, Data::Texture::Purpose::Normal);
            processTextures(pMesh, material, Data::Texture::Purpose::Height);
    }
}

    void MeshManager::processTextures(Data::Mesh& pMesh, aiMaterial* pMaterial, const Data::Texture::Purpose& pPurpose)
    {
        aiTextureType type = aiTextureType::aiTextureType_UNKNOWN;
        switch (pPurpose)
        {
                case Data::Texture::Purpose::Diffuse: type = aiTextureType_DIFFUSE; break;
                case Data::Texture::Purpose::Specular: type = aiTextureType_SPECULAR; break;
                case Data::Texture::Purpose::Normal: type = aiTextureType_HEIGHT; break;
                case Data::Texture::Purpose::Height: type = aiTextureType_AMBIENT; break;
        default:
            ZEPHYR_ASSERT(false, "This Texture::Purpose has no corresponding ASSIMP type.");
            break;
        }

        for (unsigned int i = 0; i < pMaterial->GetTextureCount(type); i++)
        {
            aiString fileName;
            pMaterial->GetTexture(type, i, &fileName);
            const std::string textureFilePath = pMesh.mFilePath + "/" + fileName.C_Str();
                TextureID tex                     = mTextureManager.loadTexture(textureFilePath, pPurpose).mID;
            pMesh.mTextures.push_back(tex);
        }
    }

    bool MeshManager::isMeshValid(const Data::Mesh& pMesh)
    {
        ZEPHYR_ASSERT(!pMesh.mName.empty(), "Mesh name cannot be empty.");

        // This check will only check leaf nodes since a child could have more children.
        // isMeshValid should take a bool isRoot.
        if (pMesh.mChildMeshes.empty())
        {
            ZEPHYR_ASSERT(!pMesh.mVertices.empty(), "Mesh position data cannot be empty");

            if (!pMesh.mNormals.empty())
                ZEPHYR_ASSERT(pMesh.mNormals.size() == pMesh.mVertices.size(), "Normal data needs to be the same size as position data");
            if (!pMesh.mColours.empty())
                ZEPHYR_ASSERT(pMesh.mColours.size() == pMesh.mVertices.size(), "Colour data needs to be the same size as position data");
            if (!pMesh.mTextureCoordinates.empty())
                ZEPHYR_ASSERT((static_cast<float>(pMesh.mVertices.size()) / static_cast<float>(pMesh.mTextureCoordinates.size())) == 1.5f, "2D Texture data needs to be at 2:3 ratio with position data");
        }

        if (!pMesh.mChildMeshes.empty()) // Check all the children are valid
        {
            for (const auto& child : pMesh.mChildMeshes)
            {
                ZEPHYR_ASSERT(pMesh.mID.Get() == child.mID.Get(), "Children should have the same mID as parents.");

                if (!isMeshValid(child))
                    return false;
            }
        }

        return true;
    }

    void MeshManager::buildMeshes() // Populates mMeshes with some commonly used shapes
    {
        loadModel("C:/Users/micha/OneDrive/Desktop/Zephyr/source/Resources/Models/xian/xian.obj");
        loadModel("C:/Users/micha/OneDrive/Desktop/Zephyr/source/Resources/Models/backpack/backpack.obj");
        loadModel("C:/Users/micha/OneDrive/Desktop/Zephyr/source/Resources/Models/cube/cube.obj");

        { // 2D TRIANGLE
            Data::Mesh mesh;
            mesh.mName = "2DTriangle";
            mesh.mVertices = {
                -1.0f, -1.0f, 0.0f, // Left
                1.0f, -1.0f, 0.0f,  // Right
                0.0f, 1.0f, 0.0f,   // Top
            };
            mesh.mColours = {
                0.0f, 1.0f, 0.0f,
                0.0f, 0.0f, 1.0f,
                1.0f, 0.0f, 0.0f};
            mesh.mTextureCoordinates = {
                0.0f, 0.0f,
                1.0f, 0.0f,
                0.5f, 1.0f};
            addMesh(mesh);
        }
        { // SKYBOX
            Data::Mesh mesh;
            mesh.mName = "Skybox";
            mesh.mVertices = {
                -1.0f,  1.0f, -1.0f,
                -1.0f, -1.0f, -1.0f,
                1.0f, -1.0f, -1.0f,
                1.0f, -1.0f, -1.0f,
                1.0f,  1.0f, -1.0f,
                -1.0f,  1.0f, -1.0f,
                -1.0f, -1.0f,  1.0f,
                -1.0f, -1.0f, -1.0f,
                -1.0f,  1.0f, -1.0f,
                -1.0f,  1.0f, -1.0f,
                -1.0f,  1.0f,  1.0f,
                -1.0f, -1.0f,  1.0f,
                1.0f, -1.0f, -1.0f,
                1.0f, -1.0f,  1.0f,
                1.0f,  1.0f,  1.0f,
                1.0f,  1.0f,  1.0f,
                1.0f,  1.0f, -1.0f,
                1.0f, -1.0f, -1.0f,
                -1.0f, -1.0f,  1.0f,
                -1.0f,  1.0f,  1.0f,
                1.0f,  1.0f,  1.0f,
                1.0f,  1.0f,  1.0f,
                1.0f, -1.0f,  1.0f,
                -1.0f, -1.0f,  1.0f,
                -1.0f,  1.0f, -1.0f,
                1.0f,  1.0f, -1.0f,
                1.0f,  1.0f,  1.0f,
                1.0f,  1.0f,  1.0f,
                -1.0f,  1.0f,  1.0f,
                -1.0f,  1.0f, -1.0f,
                -1.0f, -1.0f, -1.0f,
                -1.0f, -1.0f,  1.0f,
                1.0f, -1.0f, -1.0f,
                1.0f, -1.0f, -1.0f,
                -1.0f, -1.0f,  1.0f,
                1.0f, -1.0f,  1.0f
            };
            addMesh(mesh);
        }
        { // QUAD
            Data::Mesh mesh;
            mesh.mName = "Quad";
            mesh.mVertices = {
            -1.0f,  1.0f, 0.0f,  // Top-left
            -1.0f, -1.0f, 0.0f,  // Bottom-left
                1.0f, -1.0f, 0.0f,  // Bottom-right
                1.0f,  1.0f, 0.0f}; // Top-right
            mesh.mTextureCoordinates = {
                0.0f, 1.0f,
                0.0f, 0.0f,
                1.0f, 0.0f,
                1.0f, 1.0f};
            mesh.mNormals = {
                0.0f, 0.0f, 1.f,
                0.0f, 0.0f, 1.f,
                0.0f, 0.0f, 1.f,
                0.0f, 0.0f, 1.f};
            mesh.mIndices = {
                0, 1, 2,
                0, 2, 3};

            addMesh(mesh);
        }
        { // 3D CUBE (supported vertex attributes: Position, Texture coordinates(2D), Normal)
            Data::Mesh mesh;
            mesh.mName = "3DCube";
            mesh.mVertices = {
                // Back face
                -0.5f, -0.5f, -0.5f, // Bottom-left
                0.5f,  0.5f, -0.5f, // top-right
                0.5f, -0.5f, -0.5f, // bottom-right
                0.5f,  0.5f, -0.5f, // top-right
                -0.5f, -0.5f, -0.5f, // bottom-left
                -0.5f,  0.5f, -0.5f, // top-left
                // Front face
                -0.5f, -0.5f,  0.5f, // bottom-left
                0.5f, -0.5f,  0.5f, // bottom-right
                0.5f,  0.5f,  0.5f, // top-right
                0.5f,  0.5f,  0.5f, // top-right
                -0.5f,  0.5f,  0.5f, // top-left
                -0.5f, -0.5f,  0.5f, // bottom-left
                // Left face
                -0.5f,  0.5f,  0.5f, // top-right
                -0.5f,  0.5f, -0.5f, // top-left
                -0.5f, -0.5f, -0.5f, // bottom-left
                -0.5f, -0.5f, -0.5f, // bottom-left
                -0.5f, -0.5f,  0.5f, // bottom-right
                -0.5f,  0.5f,  0.5f, // top-right
                // Right face
                0.5f,  0.5f,  0.5f, // top-left
                0.5f, -0.5f, -0.5f, // bottom-right
                0.5f,  0.5f, -0.5f, // top-right
                0.5f, -0.5f, -0.5f, // bottom-right
                0.5f,  0.5f,  0.5f, // top-left
                0.5f, -0.5f,  0.5f, // bottom-left
                // Bottom face
                -0.5f, -0.5f, -0.5f, // top-right
                0.5f, -0.5f, -0.5f, // top-left
                0.5f, -0.5f,  0.5f, // bottom-left
                0.5f, -0.5f,  0.5f, // bottom-left
                -0.5f, -0.5f,  0.5f, // bottom-right
                -0.5f, -0.5f, -0.5f, // top-right
                // Top face
                -0.5f,  0.5f, -0.5f, // top-left
                0.5f,  0.5f,  0.5f, // bottom-right
                0.5f,  0.5f, -0.5f, // top-right
                0.5f,  0.5f,  0.5f, // bottom-right
                -0.5f,  0.5f, -0.5f, // top-left
                -0.5f,  0.5f,  0.5f  // bottom-left
            };
            mesh.mNormals = {
                // Back face
                0.0f, 0.0f, -1.f, // Bottom-left
                0.0f, 0.0f, -1.f, // top-right
                0.0f, 0.0f, -1.f, // bottom-right
                0.0f, 0.0f, -1.f, // top-right
                0.0f, 0.0f, -1.f, // bottom-left
                0.0f, 0.0f, -1.f, // top-left
                // Front face
                0.0f, 0.0f, 1.f, // bottom-left
                0.0f, 0.0f, 1.f, // bottom-right
                0.0f, 0.0f, 1.f, // top-right
                0.0f, 0.0f, 1.f, // top-right
                0.0f, 0.0f, 1.f, // top-left
                0.0f, 0.0f, 1.f, // bottom-left
                // Left face
                -1.0f, 0.0f, 0.f, // top-right
                -1.0f, 0.0f, 0.f, // top-left
                -1.0f, 0.0f, 0.f, // bottom-left
                -1.0f, 0.0f, 0.f, // bottom-left
                -1.0f, 0.0f, 0.f, // bottom-right
                -1.0f, 0.0f, 0.f, // top-right
                // Right face
                1.0f, 0.0f, 0.f, // top-left
                1.0f, 0.0f, 0.f, // bottom-right
                1.0f, 0.0f, 0.f, // top-right
                1.0f, 0.0f, 0.f, // bottom-right
                1.0f, 0.0f, 0.f, // top-left
                1.0f, 0.0f, 0.f, // bottom-left
                // Bottom face
                0.0f, -1.0f, 0.f, // top-right
                0.0f, -1.0f, 0.f, // top-left
                0.0f, -1.0f, 0.f, // bottom-left
                0.0f, -1.0f, 0.f, // bottom-left
                0.0f, -1.0f, 0.f, // bottom-right
                0.0f, -1.0f, 0.f, // top-right
                // Top face
                0.0f, 1.0f, 0.f, // top-left
                0.0f, 1.0f, 0.f, // bottom-right
                0.0f, 1.0f, 0.f, // top-right
                0.0f, 1.0f, 0.f, // bottom-right
                0.0f, 1.0f, 0.f, // top-left
                0.0f, 1.0f, 0.f, // bottom-left
                };
            mesh.mTextureCoordinates = {
                0.0f, 0.0f, // Bottom-left
                1.0f, 1.0f, // top-right
                1.0f, 0.0f, // bottom-right
                1.0f, 1.0f, // top-right
                0.0f, 0.0f, // bottom-left
                0.0f, 1.0f, // top-left
                0.0f, 0.0f, // bottom-left
                1.0f, 0.0f, // bottom-right
                1.0f, 1.0f, // top-right
                1.0f, 1.0f, // top-right
                0.0f, 1.0f, // top-left
                0.0f, 0.0f, // bottom-left
                1.0f, 0.0f, // top-right
                1.0f, 1.0f, // top-left
                0.0f, 1.0f, // bottom-left
                0.0f, 1.0f, // bottom-left
                0.0f, 0.0f, // bottom-right
                1.0f, 0.0f, // top-right
                1.0f, 0.0f, // top-left
                0.0f, 1.0f, // bottom-right
                1.0f, 1.0f, // top-right
                0.0f, 1.0f, // bottom-right
                1.0f, 0.0f, // top-left
                0.0f, 0.0f, // bottom-left
                0.0f, 1.0f, // top-right
                1.0f, 1.0f, // top-left
                1.0f, 0.0f, // bottom-left
                1.0f, 0.0f, // bottom-left
                0.0f, 0.0f, // bottom-right
                0.0f, 1.0f, // top-right
                0.0f, 1.0f, // top-left
                1.0f, 0.0f, // bottom-right
                1.0f, 1.0f, // top-right
                1.0f, 0.0f, // bottom-right
                0.0f, 1.0f, // top-left
                0.0f, 0.0f  // bottom-left
            };
            addMesh(mesh);
        }
    }
} // namespace Manager