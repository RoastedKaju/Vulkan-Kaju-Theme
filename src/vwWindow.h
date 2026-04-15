#pragma once

#include "vwCommon.h"

namespace vw
{
    class Instance;

    class Window
    {
    public:
        Window() = default;
        ~Window() = default;

        void createWindow(const int width, const int height);
        void destroyWindow();

        void draw();

        void createSurface(Instance& instance);
        void destroySurface(Instance& instance);

        inline GLFWwindow *getWindow() { return window; }
        inline VkExtent2D getWindowExtent() const { return extent; }
        inline VkSurfaceKHR getSurface() { return surface; }
        inline void setSurface(VkSurfaceKHR inSurface) { surface = inSurface; }

    private:
        GLFWwindow *window;
        VkExtent2D extent;
        VkSurfaceKHR surface;
    };
}
