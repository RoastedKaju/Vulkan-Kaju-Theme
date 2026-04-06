#pragma once

#include "common_types.h"

class KajuWindow
{
public:
    KajuWindow(uint32_t width, uint32_t height);
    ~KajuWindow();

    void createSurface(VkInstance instance);
    void destroySurface(VkInstance instance);

    inline VkSurfaceKHR getSurface() const { return surface; }
    inline GLFWwindow *getWindow() const { return window; }

private:
    GLFWwindow *window;
    VkExtent2D extent;
    VkSurfaceKHR surface;
};