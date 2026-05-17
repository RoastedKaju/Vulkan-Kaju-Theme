#include "vwWindow.h"
#include "vwInstance.h"
#include "vwSwapchain.h"

void vw::Window::createWindow(const int width, const int height)
{
    if (!glfwInit())
    {
        std::cerr << "Failed to initialize GLFW\n";
        exit(1);
    }

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
    glfwWindowHint(GLFW_MAXIMIZED, GLFW_TRUE);

    window = glfwCreateWindow(width, height, "Kaju-Theme", nullptr, nullptr);
    if (!window)
    {
        glfwTerminate();
        std::cerr << "Failed to create Window.\n";
        exit(1);
    }

    glfwSetWindowUserPointer(window, this);
    glfwSetFramebufferSizeCallback(window, &onWindowResizedCallback);

    std::cout << "Window created\n";
}

void vw::Window::destroyWindow()
{
    glfwDestroyWindow(window);
    glfwTerminate();
    std::cout << "Window destroyed\n";
}

void vw::Window::createSurface(Instance &instance)
{
    VW_CHECK(glfwCreateWindowSurface(instance.getInstance(), window, nullptr, &surface));
    std::cout << "Surface created\n";
}

void vw::Window::destroySurface(Instance &instance)
{
    vkDestroySurfaceKHR(instance.getInstance(), surface, nullptr);
    std::cout << "Surface Destroyed\n";
}
