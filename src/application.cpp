#include "vwInstance.h"
#include "vwWindow.h"
#include "vwSwapchain.h"
#include "vwAllocator.h"
#include "vwRenderTarget.h"
#include "vwGUI.h"
#include "vwUtils.h"

#define VMA_IMPLEMENTATION
#include <vk_mem_alloc.h>

int main()
{
    vw::Window mainWindow{};
    vw::Instance instance{};
    vw::Allocator allocator{};
    vw::Swapchain swapchain{};
    vw::RenderTarget renderTarget{};
    vw::GUI gui{};

    instance.createInstance();
    mainWindow.createWindow(800, 600);
    mainWindow.createSurface(instance);
    instance.createDevice(mainWindow);
    allocator.createAllocator(instance);
    swapchain.createSwapchain(instance, mainWindow);
    swapchain.createSyncStructures(instance);
    renderTarget.createRenderTarget({800, 600}, allocator, instance);
    // builder context object
    vw::utils::Context renderingContext{};
    renderingContext.instance = instance.getInstance();
    renderingContext.physicalDevice = instance.getPhysicalDevice();
    renderingContext.device = instance.getDevice();
    renderingContext.graphicsQueue = instance.getGraphicsQueue();
    renderingContext.graphicsQueueFamily = instance.getGraphicsQueueFamily();
    renderingContext.swapchain = swapchain.getSwapchain();
    renderingContext.swapchainFormat = swapchain.getFormat();
    renderingContext.swapchainImageCount = swapchain.getImages().size();
    renderingContext.drawImageView = renderTarget.getDrawImageView();

    gui.createGUIContext(mainWindow.getWindow(), renderingContext);

    // draw loop
    while (!glfwWindowShouldClose(mainWindow.getWindow()))
    {
        glfwPollEvents();

        // skip frame if invalid frame size
        int width = 0, height = 0;
        glfwGetFramebufferSize(mainWindow.getWindow(), &width, &height);
        if (!width || !height)
        {
            continue;
        }

        vkDeviceWaitIdle(instance.getDevice());

        VW_CHECK(vkWaitForFences(instance.getDevice(), 1, &swapchain.getCurrentFrameData().renderFence, true, 1000000000));
        swapchain.getCurrentFrameData().frameDeletionQueue.flush();
        VW_CHECK(vkResetFences(instance.getDevice(), 1, &swapchain.getCurrentFrameData().renderFence));

        uint32_t swapchainImageIndex;
        auto acquireResult = vkAcquireNextImageKHR(instance.getDevice(), swapchain.getSwapchain(), 1000000000, swapchain.getCurrentFrameData().acquireSemaphore, nullptr, &swapchainImageIndex);
        if (acquireResult == VK_ERROR_OUT_OF_DATE_KHR)
        {
            // Swapchain is stale, recreate and retry next frame
            int w = 0, h = 0;
            glfwGetFramebufferSize(mainWindow.getWindow(), &w, &h);
            swapchain.createSwapchain(instance, mainWindow);
            swapchain.incrementFrame();
            continue;
        }
        if (acquireResult != VK_SUCCESS && acquireResult != VK_SUBOPTIMAL_KHR)
        {
            VW_CHECK(acquireResult); // only fatal on real errors
        }

        VkCommandBuffer cmd = swapchain.getCurrentFrameData().cmdBuffer;
        VW_CHECK(vkResetCommandBuffer(cmd, 0));

        VkCommandBufferBeginInfo cmdBeginInfo = vw::utils::cmdBufferBeginInfo(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
        VW_CHECK(vkBeginCommandBuffer(cmd, &cmdBeginInfo));
        {
            // vw::utils::transitionImage(cmd, swapchain.getImages().at(swapchainImageIndex), VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);
            // clear color
            // float flash = std::abs(std::sin((float)glfwGetTime() * 2.0f));
            // VkClearColorValue clearValue = {{0.0f, 0.0f, flash, 1.0f}};
            // VkImageSubresourceRange clearRange = vw::utils::imageSubresourceRange(VK_IMAGE_ASPECT_COLOR_BIT);
            // vkCmdClearColorImage(cmd, swapchain.getImages().at(swapchainImageIndex), VK_IMAGE_LAYOUT_GENERAL, &clearValue, 1, &clearRange);

            vw::utils::transitionImage(cmd, swapchain.getImages().at(swapchainImageIndex), VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
            gui.beginFrame();
            {
                // ImGui::Begin("Demo");
                // ImGui::Text("Hello World!");
                // ImGui::End();
                ImGui::ShowDemoWindow();
            }
            gui.endFrame(cmd, swapchain, swapchainImageIndex);
            vw::utils::transitionImage(cmd, swapchain.getImages().at(swapchainImageIndex), VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
        }
        VW_CHECK(vkEndCommandBuffer(cmd));

        // present and submit
        VkCommandBufferSubmitInfo cmdInfo = vw::utils::cmdBufferSubmitInfo(cmd);
        VkSemaphoreSubmitInfo waitInfo = vw::utils::semaphoreSubmitInfo(VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT_KHR, swapchain.getCurrentFrameData().acquireSemaphore);
        VkSemaphoreSubmitInfo signalInfo = vw::utils::semaphoreSubmitInfo(VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT, swapchain.getReleaseSemaphores()[swapchainImageIndex]);

        VkSubmitInfo2 submit = vw::utils::submitInfo(&cmdInfo, &signalInfo, &waitInfo);
        VW_CHECK(vkQueueSubmit2(instance.getGraphicsQueue(), 1, &submit, swapchain.getCurrentFrameData().renderFence));

        VkPresentInfoKHR presentInfo = {VK_STRUCTURE_TYPE_PRESENT_INFO_KHR};
        auto sc = swapchain.getSwapchain();
        presentInfo.pSwapchains = &sc;
        presentInfo.swapchainCount = 1;
        presentInfo.pWaitSemaphores = &swapchain.getReleaseSemaphores()[swapchainImageIndex];
        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pImageIndices = &swapchainImageIndex;

        VkResult presentResult = vkQueuePresentKHR(instance.getGraphicsQueue(), &presentInfo);

        if (presentResult == VK_ERROR_OUT_OF_DATE_KHR || presentResult == VK_SUBOPTIMAL_KHR || mainWindow.bResized)
        {
            vkDeviceWaitIdle(instance.getDevice());
            // recreate swapchain
            int width = 0, height = 0;
            glfwGetFramebufferSize(mainWindow.getWindow(), &width, &height);
            swapchain.createSwapchain(instance, mainWindow);
            mainWindow.bResized = false;
            continue;
        }

        VW_CHECK(presentResult);

        swapchain.incrementFrame();
    }

    // wait before destroying
    vkDeviceWaitIdle(instance.getDevice());

    gui.destroyGUIContext(instance.getDevice());
    renderTarget.destroyRenderTarget(instance, allocator);
    swapchain.destroySyncStructures(instance);
    swapchain.destroySwapchain(instance);
    allocator.destroyAllocator();
    instance.destroyDevice();
    mainWindow.destroySurface(instance);
    mainWindow.destroyWindow();
    instance.destroyInstance();

    return EXIT_SUCCESS;
}