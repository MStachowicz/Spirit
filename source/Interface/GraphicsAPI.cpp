#include "GraphicsAPI.hpp"

// ECS
#include "Entity.hpp"
#include "EntityManager.hpp"
#include "Transform.hpp"

// Utilities
#include "Utility.hpp"

void GraphicsAPI::draw()
{
    for (const auto& drawCall : mDrawCalls)
        if (!drawCall.mModels.empty())
            draw(drawCall);
}

void GraphicsAPI::onEntityAdded(const ECS::Entity& pEntity, const ECS::EntityManager& pManager)
{
    // If an entity has a MeshDraw and Transform component, add it to the mDrawCalls list.
    // If the MeshDraw variation already exists in a DrawCall, append just the Transform data to the mModels.
    if (const Data::MeshDraw* mesh = pManager.mMeshes.GetComponent(pEntity))
    {
        if (const Data::Transform* transform = pManager.mTransforms.GetComponent(pEntity))
        {
            auto it = std::find_if(mDrawCalls.begin(), mDrawCalls.end(), [mesh](const DrawCall& entry)
            {
                return entry.mMesh.mID == mesh->mID
                && entry.mMesh.mDrawMode == mesh->mDrawMode
                && entry.mMesh.mDrawStyle == mesh->mDrawStyle
                // Per DrawStyle values
                && entry.mMesh.mTexture1 == mesh->mTexture1
                && entry.mMesh.mTexture2 == mesh->mTexture2
                && entry.mMesh.mMixFactor == mesh->mMixFactor
                && entry.mMesh.mColour == mesh->mColour
                && entry.mMesh.mDiffuseTextureID == mesh->mDiffuseTextureID
                && entry.mMesh.mSpecularTextureID == mesh->mSpecularTextureID
                && entry.mMesh.mShininess == mesh->mShininess
                && entry.mMesh.mTextureRepeatFactor == mesh->mTextureRepeatFactor; });

            if (it == mDrawCalls.end())
            {
                DrawCall drawCall;
                drawCall.mMesh                                = *mesh;
                drawCall.mEntityModelIndexLookup[pEntity.mID] = drawCall.mModels.size();
                drawCall.mModels.push_back(util::GetModelMatrix(transform->mPosition, transform->mRotation, transform->mScale));
                mDrawCalls.push_back(drawCall);
            }
            else
            {
                it->mEntityModelIndexLookup[pEntity.mID] = it->mModels.size();
                it->mModels.push_back(util::GetModelMatrix(transform->mPosition, transform->mRotation, transform->mScale));
            }
        }
    }
}

void GraphicsAPI::onTransformComponentChange(const ECS::Entity& pEntity, const Data::Transform& pTransform)
{
    // Find the DrawCall containing pEntity Transform data and update the model matrix for it.
    for (size_t i = 0; i < mDrawCalls.size(); i++)
    {
        auto it = mDrawCalls[i].mEntityModelIndexLookup.find(pEntity.mID);
        if (it != mDrawCalls[i].mEntityModelIndexLookup.end())
        {
            mDrawCalls[i].mModels[it->second] = util::GetModelMatrix(pTransform.mPosition, pTransform.mRotation, pTransform.mScale);
            return;
        }
    }
}