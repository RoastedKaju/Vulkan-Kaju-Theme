#include "kaju_window.h"

#include "Instance.h"

KajuWindow::KajuWindow(uint32_t width, uint32_t height)
    : window(nullptr),
      extent(),
      surface(VK_NULL_HANDLE)
{
    std::cout << "Creating GLFW window.\n";

    if (!glfwInit())
    {
        throw std::runtime_error("Failed to initialize GLFW.\n");
    }

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

    extent.width = width;
    extent.height = height;

    window = glfwCreateWindow(extent.width, extent.height, "Kaju Theme Window", nullptr, nullptr);
}

KajuWindow::~KajuWindow()
{
    std::cout << "Destroying GLFW Window.\n";

    glfwDestroyWindow(window);
    glfwTerminate();
}

void KajuWindow::createSurface(Instance &instance)
{
    if (glfwCreateWindowSurface(instance.getInstance(), window, nullptr, &surface) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create window surface.\n");
    }
}

void KajuWindow::destroySurface(Instance &instance)
{
    vkDestroySurfaceKHR(instance.getInstance(), surface, nullptr);
}
