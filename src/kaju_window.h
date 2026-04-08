#pragma once

#include "common_types.h"

class Instance;

class KajuWindow
{
public:
    KajuWindow(uint32_t width, uint32_t height);
    ~KajuWindow();

    void createSurface(Instance &instance);
    void destroySurface(Instance &instance);

    inline VkSurfaceKHR getSurface() const { return surface; }
    inline GLFWwindow *getWindow() const { return window; }

    bool frameBufferResized = false;

private:
    GLFWwindow *window;
    VkExtent2D extent;
    VkSurfaceKHR surface;
};

static void frameResizeCallback(GLFWwindow *window, int width, int height)
{
    auto kaju_window_ptr = reinterpret_cast<KajuWindow *>(glfwGetWindowUserPointer(window));
    kaju_window_ptr->frameBufferResized = true;
}