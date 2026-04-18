#include "vwConsole.h"

#include <algorithm>
#include <cstring>

vw::Console::Console()
{
}

void vw::Console::showConsole(ImGuiID parentId)
{
    static bool firstRun = true;
    if (firstRun && parentId != 0)
    {
        elementId = ImGui::GetID("Console");
        firstRun = false;
    }

    if (!ImGui::Begin("Console"))
    {
        ImGui::End();
        return;
    }

    drawToolBar();
    ImGui::Separator();
    drawLog();
    ImGui::Separator();
    drawInputBar();

    ImGui::End();
}

void vw::Console::ClearLog()
{
    entries.clear();
}

void vw::Console::AddLog(std::string_view message)
{
    pushEntry(message);
}

void vw::Console::drawToolBar()
{
    filter.Draw("Filter", 180.0f);

    ImGui::SameLine();
    if (ImGui::Button("Clear"))
    {
        ClearLog();
    }

    ImGui::SameLine();
    if (ImGui::Button("Copy"))
    {
        ImGui::LogToClipboard();
        for (const auto &entry : entries)
        {
            if (filter.PassFilter(entry.text.c_str()))
            {
                ImGui::LogText("%s\n", entry.text.c_str());
            }
        }
        ImGui::LogFinish();
    }

    ImGui::SameLine();
    ImGui::Checkbox("Auto-scroll", &autoScroll);
}

void vw::Console::drawLog()
{
    const float footerHeight = ImGui::GetStyle().ItemSpacing.y + ImGui::GetFrameHeightWithSpacing();
    ImGui::BeginChild("##log", ImVec2(0.0f, -footerHeight), false, ImGuiWindowFlags_HorizontalScrollbar);

    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4.0f, 1.0f));

    for (const auto &entry : entries)
    {
        if (!filter.PassFilter(entry.text.c_str()))
        {
            continue;
        }

        ImGui::PushStyleColor(ImGuiCol_Text, entry.color);
        ImGui::TextUnformatted(entry.text.c_str());
        ImGui::PopStyleColor();
    }

    if (scrollToBottom || (autoScroll && ImGui::GetScrollY() >= ImGui::GetScrollMaxY()))
    {
        ImGui::SetScrollHereY(1.0f);
    }
    scrollToBottom = false;

    ImGui::PopStyleVar();
    ImGui::EndChild();
}

void vw::Console::drawInputBar()
{
    bool reclaimFocus = false;

    ImGuiInputTextFlags flags;
    flags |= ImGuiInputTextFlags_EnterReturnsTrue;
    flags |= ImGuiInputTextFlags_CallbackCompletion;
    flags |= ImGuiInputTextFlags_CallbackHistory;

    ImGui::SetNextItemWidth(-1.0f);

    if (ImGui::InputText("##input", inputBuf, sizeof(inputBuf), 0, &Console::inputCallback, this))
    {
        // trim leading spaces
        std::string_view cmd = inputBuf;
        const auto start = cmd.find_first_not_of(' ');

        if (start != std::string_view::npos)
        {
            execCommand(cmd.substr(start));
        }

        std::memset(inputBuf, 0, sizeof(inputBuf));
        reclaimFocus = true;
    }

    ImGui::SetItemDefaultFocus();
    if (reclaimFocus)
    {
        ImGui::SetKeyboardFocusHere(-1);
    }
}

void vw::Console::pushEntry(std::string_view text)
{
    entries.push_back({std::string(text), classifyColor(text)});
    scrollToBottom = true;
}

void vw::Console::execCommand(std::string_view cmd)
{
}

ImVec4 vw::Console::classifyColor(std::string_view text)
{
    if (text.starts_with("[error]"))
        return {1.0f, 0.4f, 0.4f, 1.0f};
    if (text.starts_with("[warn]"))
        return {1.0f, 0.85f, 0.2f, 1.0f};
    if (text.starts_with('#'))
        return {1.0f, 0.8f, 0.6f, 1.0f};
    return {1.0f, 1.0f, 1.0f, 1.0f};
}

int vw::Console::inputCallback(ImGuiInputTextCallbackData *data)
{
    return static_cast<Console *>(data->UserData)->onInputEvent(data);
}

int vw::Console::onInputEvent(ImGuiInputTextCallbackData *data)
{
    return 0;
}
