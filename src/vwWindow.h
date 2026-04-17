#pragma once

#include "vwCommon.h"
#include "vwUtils.h"

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

        void createSurface(Instance &instance);
        void destroySurface(Instance &instance);

        inline GLFWwindow *getWindow() { return window; }
        inline VkExtent2D getWindowExtent() const { return extent; }
        inline VkSurfaceKHR getSurface() { return surface; }
        inline void setSurface(VkSurfaceKHR inSurface) { surface = inSurface; }

        bool bResized = false;

    private:
        GLFWwindow *window;
        VkExtent2D extent;
        VkSurfaceKHR surface;
    };
}

static void onWindowResizedCallback(GLFWwindow *window, int width, int height)
{
    auto windowClass = reinterpret_cast<vw::Window *>(glfwGetWindowUserPointer(window));
    windowClass->bResized = true;
}
