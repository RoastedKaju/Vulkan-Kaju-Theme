#include "vwDockspace.h"

void vw::Dockspace::initLayouts()
{
    ImGui::DockBuilderRemoveNode(elementId);
    ImGui::DockBuilderAddNode(elementId, ImGuiDockNodeFlags_DockSpace);

    ImGui::DockBuilderSetNodeSize(elementId, ImGui::GetMainViewport()->Size);

    // split
    ImGuiID viewportID;
    ImGuiID consoleID = ImGui::DockBuilderSplitNode(elementId, ImGuiDir_Down, 0.35f, nullptr, &viewportID);

    ImGui::DockBuilderDockWindow("Viewport", viewportID);
    ImGui::DockBuilderDockWindow("Console", consoleID);

    ImGui::DockBuilderFinish(elementId);
}

void vw::Dockspace::showDockspace()
{
    elementId = ImGui::DockSpaceOverViewport(0, nullptr, ImGuiDockNodeFlags_None);

    if (initLayoutNextFrame)
    {
        initLayouts();
        initLayoutNextFrame = false;
    }
}