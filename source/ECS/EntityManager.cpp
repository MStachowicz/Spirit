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
                const std::string title = "Entity " + std::to_string(mEntities[i].mID);
                if (ImGui::TreeNode(title.c_str()))
                {
                    if (auto* component = mTransforms.GetComponentModify(mEntities[i]))
                    {
                        component->DrawImGui();
                        ImGui::Separator();
                    }
                    if (auto* component = mMeshes.GetComponentModify(mEntities[i]))
                    {
                        component->DrawImGui();
                        ImGui::Separator();
                    }
                    if (auto* component = mPointLights.GetComponentModify(mEntities[i]))
                    {
                        component->DrawImGui();
                        ImGui::Separator();
                    }
                    if (auto* component = mSpotLights.GetComponentModify(mEntities[i]))
                    {
                        component->DrawImGui();
                        ImGui::Separator();
                    }
                    if (auto* component = mDirectionalLights.GetComponentModify(mEntities[i]))
                    {
                        component->DrawImGui();
                        ImGui::Separator();
                    }

                    ImGui::TreePop();
                }
            };
        }
        ImGui::End();
    }
}