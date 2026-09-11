#pragma once

#include <context.hpp>
#include <GLFW/glfw3.h>

class Application
{
public:
    bool init();

    void run();

    void shutdown();

    GLFWwindow *pWindow{nullptr};
    // size
    int mWidth{800};
    int mHeight{600};
    // vulkan object
    Context mContext;
};