#pragma once

#include "vwCommon.h"
#include "vwGUI.h"

namespace vw
{
    class Console : public GuiElement
    {
    public:
        Console();
        virtual ~Console() override = default;

        Console(const Console &) = delete;
        Console &operator=(const Console &) = delete;
        Console(Console &&) = default;
        Console &operator=(Console &&) = default;

        void showConsole(ImGuiID parentId);

        void ClearLog();

        template <typename... Args>
        void AddLog(std::string_view fmt, Args &&...args)
        {
            char buf[1024];
            std::snprintf(buf, sizeof(buf), fmt.data(), std::forward<Args>(args)...);
            pushEntry(buf);
        }

        void AddLog(std::string_view message);

    private:
        struct Entry
        {
            std::string text;
            ImVec4 color;
        };

        std::vector<Entry> entries;
        std::vector<std::string> history;
        int historyPos = -1;
        bool autoScroll = true;
        bool scrollToBottom = true;
        char inputBuf[512] = {};
        ImGuiTextFilter filter;

        void drawToolBar();
        void drawLog();
        void drawInputBar();

        void pushEntry(std::string_view text);
        void execCommand(std::string_view cmd);

        static ImVec4 classifyColor(std::string_view text);

        static int inputCallback(ImGuiInputTextCallbackData *data);
        int onInputEvent(ImGuiInputTextCallbackData *data);
    };
}