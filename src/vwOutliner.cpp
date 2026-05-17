#include "vwOutliner.h"

void vw::Outliner::showOutliner(ImGuiID parentId)
{
    static bool firstRun = true;
    if (firstRun && parentId != 0)
    {
        elementId = ImGui::GetID("Outliner");
        firstRun = false;
    }

    ImGui::Begin("Outliner");
    ImGuiTreeNodeFlags baseFlags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanFullWidth;

    // Make the outliner scrollable
    ImGui::BeginChild("OutlinerScrollRegion", ImVec2(0, 0), false);

    if (ImGui::TreeNodeEx("Scene", baseFlags | ImGuiTreeNodeFlags_DefaultOpen))
    {
        // No child nodes
        if (ImGui::Selectable("Camera", false, ImGuiSelectableFlags_SpanAllColumns))
        {
            std::cout << "Clicked Camera" << std::endl;
        }

        if (ImGui::TreeNodeEx("Entity", baseFlags | ImGuiTreeNodeFlags_DefaultOpen))
        {
            // On click for tree nodes
            if (ImGui::IsItemClicked())
            {
                std::cout << "Clicked Entity" << std::endl;
            }
            // no child node
            if (ImGui::Selectable("Child Entity 1", false, ImGuiSelectableFlags_SpanAllColumns))
            {
                std::cout << "Clicked Child Entity 1" << std::endl;
            }
            // with child tree node
            if (ImGui::TreeNodeEx("Child Entity 2", baseFlags | ImGuiTreeNodeFlags_DefaultOpen))
            {
                if (ImGui::IsItemClicked())
                {
                    std::cout << "Clicked Child Entity 2" << std::endl;
                }

                if (ImGui::Selectable("Grandchild Entity", false, ImGuiSelectableFlags_SpanAllColumns))
                {
                    std::cout << "Clicked Grandchild Entity" << std::endl;
                }
                ImGui::TreePop();
            }

            ImGui::TreePop();
        }

        ImGui::TreePop();
    }
    ImGui::EndChild();
    ImGui::End();
}
