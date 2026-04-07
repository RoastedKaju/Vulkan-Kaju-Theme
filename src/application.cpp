#include <iostream>
#include <chrono>
#include <thread>

#include "instance.h"
#include "kaju_window.h"
#include "device.h"
#include "memory_allocator.h"
#include "swapchain.h"
#include "frame_manager.h"
#include "kaju_gui.h"
#include "renderer.h"

#define VMA_IMPLEMENTATION
#include <vk_mem_alloc.h>

int main()
{
    KajuWindow app_window{800, 600};
    Instance app_instance;
    Device app_device;
    MemoryAllocator app_allocator;
    Swapchain app_swapchain;
    FrameManager app_frame_manager{2};
    KajuGui app_gui;
    Renderer app_renderer;

    app_window.createSurface(app_instance);
    app_device.createDevice(app_instance, app_window);
    app_allocator.createAllocator(app_instance, app_device);
    app_swapchain.createSwapchain(app_device, app_window);
    app_renderer.createOffscreenRenderTarget(app_device, app_allocator, app_window);
    app_frame_manager.createFrameData(app_device);
    app_gui.createGuiContext(app_device, app_swapchain, app_window, app_instance);

    while (!glfwWindowShouldClose(app_window.getWindow()))
    {
        glfwPollEvents();

        int width = 0, height = 0;
        glfwGetFramebufferSize(app_window.getWindow(), &width, &height);

        // Skip frame if window minimized
        if (width == 0 || height == 0)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            continue;
        }

        // Draw
        app_renderer.sync(app_frame_manager, app_device, app_swapchain);
        app_renderer.recordCommands();
        app_renderer.beginRendering();
        {
            app_renderer.prepareOffscreenImage();
            // Draw 3D here (vkCmdBeginRendering and end)
            app_renderer.transitionOffscreenToShaderRead();

            app_renderer.prepareSwapchainImage();
            {
                app_gui.beginFrame();
                app_gui.buildDockingLayout();
                app_gui.showDemo();
                app_gui.endFrame(app_renderer.getCommandBuffer(), app_swapchain, app_renderer.getSwapchainImageIndex());
            }
            app_renderer.finalizeSwapchainImage();
        }
        app_renderer.endRendering();
        // Submit
        app_renderer.submit(app_device, app_swapchain, app_frame_manager);
    }

    // Clean up
    app_device.deviceWaitIdle();
    app_gui.destroyGuiContext(app_device);
    app_frame_manager.destroyFrameData(app_device);
    app_allocator.destroyAllocator();
    app_swapchain.destroySwapchain(app_device);
    app_device.destroyDevice();
    app_window.destroySurface(app_instance);
    app_instance.destroyInstance();

    return EXIT_SUCCESS;
}