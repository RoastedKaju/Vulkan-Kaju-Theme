#include "vwDockspace.h"

void vw::Dockspace::showDockspace()
{
    elementId = ImGui::DockSpaceOverViewport(0, nullptr, ImGuiDockNodeFlags_None);
}
