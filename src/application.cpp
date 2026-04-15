#include "vwInstance.h"
#include "vwWindow.h"
#include "vwSwapchain.h"
#include "vwAllocator.h"

#define VMA_IMPLEMENTATION
#include <vk_mem_alloc.h>

int main()
{
    vw::Window mainWindow{};
    vw::Instance instance{};
    vw::Allocator allocator{};
    vw::Swapchain swapchain{};

    instance.createInstance();
    mainWindow.createWindow(800, 600);
    mainWindow.createSurface(instance);
    instance.createDevice(mainWindow);
    allocator.createAllocator(instance);
    swapchain.createSwapchain(instance, mainWindow);

    mainWindow.draw();

    swapchain.destroySwapchain(instance);
    allocator.destroyAllocator();
    instance.destroyDevice();
    mainWindow.destroySurface(instance);
    mainWindow.destroyWindow();
    instance.destroyInstance();

    return EXIT_SUCCESS;
}