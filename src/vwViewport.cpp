#include "vwViewport.h"

void vw::Viewport::showViewport(ImGuiID parentId)
{
    static bool firstRun = true;
    if (firstRun)
    {
        elementId = ImGui::GetID("Viewport");
        firstRun = false;
    }

    ImGuiWindowFlags flags;
    flags |= ImGuiWindowFlags_NoBackground;
    flags |= ImGuiWindowFlags_NoCollapse;
    flags |= ImGuiWindowFlags_NoMove;

    ImGui::Begin("Viewport", 0, flags);
    ImGui::Text("Texture here");
    ImGui::End();
}
