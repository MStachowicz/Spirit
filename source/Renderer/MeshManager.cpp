#include "MeshManager.hpp"

#include "FileSystem.hpp"
#include "TextureManager.hpp"
#include "Logger.hpp"

#include "assimp/Importer.hpp"
#include "assimp/scene.h"
#include "assimp/postprocess.h"

MeshID MeshManager::getMeshID(const std::string &pMeshName)
{
    const auto it = mMeshNames.find(pMeshName);
    ZEPHYR_ASSERT(it != mMeshNames.end(), "Searching for a mesh that does not exist in Mesh store.");
    return it->second;
}

void MeshManager::setID(Mesh& pMesh, const bool& pRootMesh)
{
    pMesh.mID = pRootMesh ? activeMeshes++ : activeMeshes - 1;

    for (auto& childMesh : pMesh.mChildMeshes)
        setID(childMesh, false);
}

void MeshManager::addMesh(Mesh& pMesh)
{
    setID(pMesh, true);

    ZEPHYR_ASSERT(isMeshValid(pMesh), "Adding invalid mesh");
    ZEPHYR_ASSERT(mMeshes.find(pMesh.mID) == mMeshes.end(), "addMesh should only be called once per MeshID");
    ZEPHYR_ASSERT(mMeshNames.find(pMesh.mName) == mMeshNames.end(), "addMesh should only be called with unique mesh name");

    mMeshes.emplace(std::make_pair(pMesh.mID, pMesh));
    mMeshNames.emplace(std::make_pair(pMesh.mName, pMesh.mID));
}

MeshID MeshManager::loadModel(const std::filesystem::path& pFilePath)
{
    Assimp::Importer importer;
    const aiScene *scene = importer.ReadFile(pFilePath.string(), aiProcess_Triangulate );

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
    {
        LOG_CRITICAL("ASSIMP error loading model: \n{}", importer.GetErrorString());
        ZEPHYR_ASSERT(false, "Failed to load model using ASSIMP");
    }

    Mesh rootMesh;
    rootMesh.mName      = pFilePath.stem().string();
    rootMesh.mFilePath  = pFilePath.parent_path().string();
    processNode(rootMesh, scene->mRootNode, scene);

    addMesh(rootMesh); // Only addMesh on the root, all children are contained inside the root node in mMeshes
    return rootMesh.getID();
}

// Recursively travel all the aiNodes and extract the per-vertex data into a Zephyr mesh object
void MeshManager::processNode(Mesh& pParentMesh, aiNode* pNode, const aiScene* pScene)
{
    for (unsigned int i = 0; i < pNode->mNumMeshes; i++)
    {
        aiMesh *mesh = pScene->mMeshes[pNode->mMeshes[i]];
        processData(pParentMesh, mesh, pScene);
    }

    for (unsigned int i = 0; i < pNode->mNumChildren; i++)
    {
        Mesh childMesh;
        childMesh.mID = pParentMesh.mID; // Child meshes are not stored in the root mMeshes container, but they do repeat their parent mID for easier find.
        childMesh.mName = pParentMesh.mName + "-child-" + std::to_string(i);
        childMesh.mFilePath = pParentMesh.mFilePath;
        processNode(childMesh, pNode->mChildren[i], pScene);
        pParentMesh.mChildMeshes.push_back(childMesh);
    }
}

void MeshManager::processData(Mesh& pMesh, const aiMesh *pAssimpMesh, const aiScene *pAssimpScene)
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
        aiMaterial *material = pAssimpScene->mMaterials[pAssimpMesh->mMaterialIndex];

        processTextures(pMesh, material, Texture::Purpose::Diffuse);
        processTextures(pMesh, material, Texture::Purpose::Specular);
        processTextures(pMesh, material, Texture::Purpose::Normal);
        processTextures(pMesh, material, Texture::Purpose::Height);
    }
}

void MeshManager::processTextures(Mesh& pMesh, aiMaterial* pMaterial, const Texture::Purpose& pPurpose)
{
    aiTextureType type = aiTextureType::aiTextureType_UNKNOWN;
    switch (pPurpose)
    {
    case Texture::Purpose::Diffuse:  type = aiTextureType_DIFFUSE;  break;
    case Texture::Purpose::Specular: type = aiTextureType_SPECULAR; break;
    case Texture::Purpose::Normal:   type = aiTextureType_HEIGHT;   break;
    case Texture::Purpose::Height:   type = aiTextureType_AMBIENT;  break;
    default:
        ZEPHYR_ASSERT(false, "This Texture::Purpose has no corresponding ASSIMP type.");
        break;
    }

    for (unsigned int i = 0; i < pMaterial->GetTextureCount(type); i++)
    {
        aiString fileName;
        pMaterial->GetTexture(type, i, &fileName);
        const std::string textureFilePath = pMesh.mFilePath + "/" + fileName.C_Str();
        TextureID tex = mTextureManager.loadTexture(textureFilePath, pPurpose);
        pMesh.mTextures.push_back(tex);
    }
}

bool MeshManager::isMeshValid(const Mesh& pMesh)
{
    ZEPHYR_ASSERT(!pMesh.mName.empty(), "Mesh name cannot be empty.");

    // This check will only check leaf nodes since a child could have more children.
    // isMeshValid should take a bool isRoot.
    if (pMesh.mChildMeshes.empty())
        ZEPHYR_ASSERT(!pMesh.mVertices.empty(), "Mesh position data cannot be empty.");

    if (!pMesh.mChildMeshes.empty()) // Check all the children are valid
    {
        for (const auto& child : pMesh.mChildMeshes)
        {
            ZEPHYR_ASSERT(pMesh.mID == child.mID, "Children should have the same mID as parents.");

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

    { // 2D TRIANGLE
        Mesh mesh;
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
    { // QUAD
        Mesh mesh;
        mesh.mName = "Quad";
        mesh.mVertices = {
            -1.0f, 1.0f, 0.0f,  // Top left
            -1.0f, -1.0f, 0.0f, // Bottom left
            1.0f, -1.0f, 0.0f,  // Bottom right
            1.0f, 1.0f, 0.0f,   // Top right
        };
        mesh.mColours = {
            0.0f, 0.0f, 1.0f,
            0.0f, 1.0f, 0.0f,
            1.0f, 0.0f, 0.0f,
            1.0f, 1.0f, 0.0f};
        mesh.mTextureCoordinates = {
            1.0f, 1.0f,
            1.0f, 0.0f,
            0.0f, 0.0f,
            0.0f, 1.0f};
        mesh.mNormals = {
            0.0f, 0.0f, 1.f,
            0.0f, 0.0f, 1.f,
            0.0f, 0.0f, 1.f,
            0.0f, 0.0f, 1.f  };
        mesh.mIndices = {
            0, 1, 3, // first triangle
            1, 2, 3};  // second triangle

        addMesh(mesh);
    }
    { // 3D CUBE (supported vertex attributes: Position, Texture coordinates(2D), Normal, Colour)
        Mesh mesh;
        mesh.mName = "3DCube";
        mesh.mVertices = {
            -0.5f, -0.5f, -0.5f,
            0.5f, -0.5f, -0.5f,
            0.5f, 0.5f, -0.5f,
            0.5f, 0.5f, -0.5f,
            -0.5f, 0.5f, -0.5f,
            -0.5f, -0.5f, -0.5f,

            -0.5f, -0.5f, 0.5f,
            0.5f, -0.5f, 0.5f,
            0.5f, 0.5f, 0.5f,
            0.5f, 0.5f, 0.5f,
            -0.5f, 0.5f, 0.5f,
            -0.5f, -0.5f, 0.5f,

            -0.5f, 0.5f, 0.5f,
            -0.5f, 0.5f, -0.5f,
            -0.5f, -0.5f, -0.5f,
            -0.5f, -0.5f, -0.5f,
            -0.5f, -0.5f, 0.5f,
            -0.5f, 0.5f, 0.5f,

            0.5f, 0.5f, 0.5f,
            0.5f, 0.5f, -0.5f,
            0.5f, -0.5f, -0.5f,
            0.5f, -0.5f, -0.5f,
            0.5f, -0.5f, 0.5f,
            0.5f, 0.5f, 0.5f,

            -0.5f, -0.5f, -0.5f,
            0.5f, -0.5f, -0.5f,
            0.5f, -0.5f, 0.5f,
            0.5f, -0.5f, 0.5f,
            -0.5f, -0.5f, 0.5f,
            -0.5f, -0.5f, -0.5f,

            -0.5f, 0.5f, -0.5f,
            0.5f, 0.5f, -0.5f,
            0.5f, 0.5f, 0.5f,
            0.5f, 0.5f, 0.5f,
            -0.5f, 0.5f, 0.5f,
            -0.5f, 0.5f, -0.5f};
        mesh.mNormals = {
            0.0f, 0.0f, -1.0f,
            0.0f, 0.0f, -1.0f,
            0.0f, 0.0f, -1.0f,
            0.0f, 0.0f, -1.0f,
            0.0f, 0.0f, -1.0f,
            0.0f, 0.0f, -1.0f,
            0.0f, 0.0f, 1.0f,
            0.0f, 0.0f, 1.0f,
            0.0f, 0.0f, 1.0f,
            0.0f, 0.0f, 1.0f,
            0.0f, 0.0f, 1.0f,
            0.0f, 0.0f, 1.0f,
            -1.0f, 0.0f, 0.0f,
            -1.0f, 0.0f, 0.0f,
            -1.0f, 0.0f, 0.0f,
            -1.0f, 0.0f, 0.0f,
            -1.0f, 0.0f, 0.0f,
            -1.0f, 0.0f, 0.0f,
            1.0f, 0.0f, 0.0f,
            1.0f, 0.0f, 0.0f,
            1.0f, 0.0f, 0.0f,
            1.0f, 0.0f, 0.0f,
            1.0f, 0.0f, 0.0f,
            1.0f, 0.0f, 0.0f,
            0.0f, -1.0f, 0.0f,
            0.0f, -1.0f, 0.0f,
            0.0f, -1.0f, 0.0f,
            0.0f, -1.0f, 0.0f,
            0.0f, -1.0f, 0.0f,
            0.0f, -1.0f, 0.0f,
            0.0f, 1.0f, 0.0f,
            0.0f, 1.0f, 0.0f,
            0.0f, 1.0f, 0.0f,
            0.0f, 1.0f, 0.0f,
            0.0f, 1.0f, 0.0f,
            0.0f, 1.0f, 0.0f};
        mesh.mTextureCoordinates = {
            0.0f, 0.0f,
            1.0f, 0.0f,
            1.0f, 1.0f,
            1.0f, 1.0f,
            0.0f, 1.0f,
            0.0f, 0.0f,
            0.0f, 0.0f,
            1.0f, 0.0f,
            1.0f, 1.0f,
            1.0f, 1.0f,
            0.0f, 1.0f,
            0.0f, 0.0f,
            1.0f, 0.0f,
            1.0f, 1.0f,
            0.0f, 1.0f,
            0.0f, 1.0f,
            0.0f, 0.0f,
            1.0f, 0.0f,
            1.0f, 0.0f,
            1.0f, 1.0f,
            0.0f, 1.0f,
            0.0f, 1.0f,
            0.0f, 0.0f,
            1.0f, 0.0f,
            0.0f, 1.0f,
            1.0f, 1.0f,
            1.0f, 0.0f,
            1.0f, 0.0f,
            0.0f, 0.0f,
            0.0f, 1.0f,
            0.0f, 1.0f,
            1.0f, 1.0f,
            1.0f, 0.0f,
            1.0f, 0.0f,
            0.0f, 0.0f,
            0.0f, 1.0f};
        const glm::vec3 colour = glm::vec3(0.0f, 0.0f, 1.0f);
        mesh.mColours = {
            colour.r, colour.g, colour.b,
            colour.r, colour.g, colour.b,
            colour.r, colour.g, colour.b,
            colour.r, colour.g, colour.b,
            colour.r, colour.g, colour.b,
            colour.r, colour.g, colour.b,
            colour.r, colour.g, colour.b,
            colour.r, colour.g, colour.b,
            colour.r, colour.g, colour.b,
            colour.r, colour.g, colour.b,
            colour.r, colour.g, colour.b,
            colour.r, colour.g, colour.b,
            colour.r, colour.g, colour.b,
            colour.r, colour.g, colour.b,
            colour.r, colour.g, colour.b,
            colour.r, colour.g, colour.b,
            colour.r, colour.g, colour.b,
            colour.r, colour.g, colour.b,
            colour.r, colour.g, colour.b,
            colour.r, colour.g, colour.b,
            colour.r, colour.g, colour.b,
            colour.r, colour.g, colour.b,
            colour.r, colour.g, colour.b,
            colour.r, colour.g, colour.b,
            colour.r, colour.g, colour.b,
            colour.r, colour.g, colour.b,
            colour.r, colour.g, colour.b,
            colour.r, colour.g, colour.b,
            colour.r, colour.g, colour.b,
            colour.r, colour.g, colour.b,
            colour.r, colour.g, colour.b,
            colour.r, colour.g, colour.b,
            colour.r, colour.g, colour.b,
            colour.r, colour.g, colour.b,
            colour.r, colour.g, colour.b,
            colour.r, colour.g, colour.b};
        addMesh(mesh);
    }
}