#include "vwDockspace.h"

void vw::Dockspace::initLayouts()
{
    ImGui::DockBuilderRemoveNode(elementId);
    ImGui::DockBuilderAddNode(elementId, ImGuiDockNodeFlags_None);
    ImGui::DockBuilderSetNodeSize(elementId, ImGui::GetMainViewport()->Size);

    ImGuiID dockspaceID = elementId;
    ImGuiID outlinerID;
    ImGuiID centerID;

    outlinerID = ImGui::DockBuilderSplitNode(dockspaceID, ImGuiDir_Left, 0.2f, nullptr, &centerID);

    ImGuiID consoleID;
    ImGuiID viewportID;

    consoleID = ImGui::DockBuilderSplitNode(centerID, ImGuiDir_Down, 0.25f, nullptr, &viewportID);

    ImGui::DockBuilderDockWindow("Outliner", outlinerID);
    ImGui::DockBuilderDockWindow("Console", consoleID);
    ImGui::DockBuilderDockWindow("Viewport", viewportID);

    ImGui::DockBuilderFinish(elementId);
}

void vw::Dockspace::showDockspace()
{
    static bool firstRun = true;
    if (firstRun)
    {
        elementId = ImGui::GetID("Dockspace");
        firstRun = false;
    }

    ImGui::DockSpaceOverViewport(elementId, nullptr, ImGuiDockNodeFlags_PassthruCentralNode);

    if (initLayoutNextFrame)
    {
        initLayouts();
        initLayoutNextFrame = false;
    }
}