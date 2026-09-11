#include <application.hpp>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

static void OnKeyCallback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
    if (action == GLFW_RELEASE)
    {
        if (key == GLFW_KEY_ESCAPE)
        {
            glfwSetWindowShouldClose(window, true);
        }
    }
}

static void OnFrameResize(GLFWwindow *window, int width, int height)
{
    if (Application *app = static_cast<Application *>(glfwGetWindowUserPointer(window)))
    {
        app->mWidth = width;
        app->mHeight = height;
    }
}

bool Application::init()
{
    if (!glfwInit())
    {
        return false;
    }

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    pWindow = glfwCreateWindow(mWidth, mHeight, "Vulkan", nullptr, nullptr);
    if (!pWindow)
    {
        return false;
    }

    glfwSetWindowUserPointer(pWindow, this);
    glfwSetKeyCallback(pWindow, &OnKeyCallback);
    glfwSetFramebufferSizeCallback(pWindow, &OnFrameResize);

    mContext.init();

    return true;
}

void Application::run()
{
    while (!glfwWindowShouldClose(pWindow))
    {
        // poll events
        glfwPollEvents();
    }
}

void Application::shutdown()
{
    mContext.shutdown();

    glfwDestroyWindow(pWindow);
    pWindow = nullptr;
    glfwTerminate();
}