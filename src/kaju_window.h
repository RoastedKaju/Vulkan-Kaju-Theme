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

private:
    GLFWwindow *window;
    VkExtent2D extent;
    VkSurfaceKHR surface;
};