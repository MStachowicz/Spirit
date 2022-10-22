#include "EntityManager.hpp"
#include "imgui.h"

namespace ECS
{
    void EntityManager::DrawImGui()
    {
        if (ImGui::Begin("Entity options"))
        {
            for (size_t i = 0; i < mEntities.size(); i++)
            {
                if (mEntities[i].mActive)
                {
                    const std::string title = "Entity " + std::to_string(mEntities[i].mID);
                    if (ImGui::TreeNode(title.c_str()))
                    {
                        if (mTransforms.Modify(mEntities[i],        [](auto& component){ component.DrawImGui(); }))
                            ImGui::Separator();
                        if (mMeshes.Modify(mEntities[i],            [](auto& component){ component.DrawImGui(); }))
                            ImGui::Separator();
                        if (mPointLights.Modify(mEntities[i],       [](auto& component){ component.DrawImGui(); }))
                            ImGui::Separator();
                        if (mSpotLights.Modify(mEntities[i],        [](auto& component){ component.DrawImGui(); }))
                            ImGui::Separator();
                        if (mDirectionalLights.Modify(mEntities[i], [](auto& component){ component.DrawImGui(); }))
                            ImGui::Separator();

                        if (ImGui::Button("Delete"))
                            removeEntity(mEntities[i].mID);

                        ImGui::TreePop();
                    }
                }
            };
        }
        ImGui::End();
    }
}