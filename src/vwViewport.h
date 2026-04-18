#pragma once

#include "vwCommon.h"
#include "vwGUI.h"

namespace vw
{
    class Viewport : public GuiElement
    {
    public:
        Viewport() = default;

        void showViewport(ImGuiID parentId);

    private:
    };
}