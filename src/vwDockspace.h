#pragma once

#include "vwCommon.h"
#include "vwGUI.h"

namespace vw
{
    class Dockspace : public GuiElement
    {
    public:
        void initLayouts();
        void showDockspace();

        void setLayoutNextFrame(bool set) { initLayoutNextFrame = set; }

    private:
        bool initLayoutNextFrame = false;
    };
}