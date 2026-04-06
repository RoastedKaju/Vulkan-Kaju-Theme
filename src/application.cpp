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

#define VMA_IMPLEMENTATION
#include <vk_mem_alloc.h>

static uint32_t frame_counter = 0;

static void transitionImage(VkCommandBuffer cmd,
                            VkImage image,
                            VkImageLayout oldLayout,
                            VkImageLayout newLayout,
                            VkAccessFlags srcAccess,
                            VkAccessFlags dstAccess,
                            VkPipelineStageFlags srcStage,
                            VkPipelineStageFlags dstStage)
{
    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = oldLayout;
    barrier.newLayout = newLayout;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = image;
    barrier.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};
    barrier.srcAccessMask = srcAccess;
    barrier.dstAccessMask = dstAccess;

    vkCmdPipelineBarrier(cmd,
                         srcStage, dstStage,
                         0,
                         0, nullptr,
                         0, nullptr,
                         1, &barrier);
}

int main()
{
    KajuWindow app_window{800, 600};
    Instance app_instance;
    Device app_device;
    MemoryAllocator app_allocator;
    Swapchain app_swapchain;
    FrameManager app_frame_manager{2};
    KajuGui app_gui;

    app_window.createSurface(app_instance.getInstance());
    app_device.createDevice(app_instance.getVkbInstance(), app_window.getSurface());
    app_allocator.createAllocator(app_instance.getInstance(), app_device.getPhysicalDevice(), app_device.getDevice());
    app_swapchain.createSwapchain(app_device, app_window);
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
        FrameData &current_frame = app_frame_manager.getFrame(frame_counter);
        if (vkWaitForFences(app_device.getDevice(), 1, &current_frame.render_fence, VK_TRUE, 1000000000) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to wait for render fence.");
        }
        // Delete all resources for current frame
        current_frame.frame_deletion_queue.flush();
        // Reset fence
        if (vkResetFences(app_device.getDevice(), 1, &current_frame.render_fence) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to reset render fence.");
        }
        // Request image from swapchain
        uint32_t swapchain_image_index;
        VkResult acquire_result = vkAcquireNextImageKHR(app_device.getDevice(), app_swapchain.getSwapchain(), 1000000000, current_frame.image_available_semaphore, VK_NULL_HANDLE, &swapchain_image_index);
        if (acquire_result != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to acquire swapchain image.");
        }
        VkImage swapchain_image = app_swapchain.getImages()[swapchain_image_index];
        // Record commands
        VkCommandBuffer cmd_buffer = current_frame.command_buffer;
        // Reset the command buffer before recording commands into it
        if (vkResetCommandBuffer(cmd_buffer, 0) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to reset command buffer.");
        }
        // Begin rendering commands
        VkCommandBufferBeginInfo begin_info{};
        begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        if (vkBeginCommandBuffer(cmd_buffer, &begin_info) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to begin command buffer.");
        }

        // // Undefined -> Transfer destination
        // transitionImage(cmd_buffer, swapchain_image,
        //                 VK_IMAGE_LAYOUT_UNDEFINED,
        //                 VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        //                 0,
        //                 VK_ACCESS_TRANSFER_WRITE_BIT,
        //                 VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
        //                 VK_PIPELINE_STAGE_TRANSFER_BIT);

        // VkClearColorValue clear_color{};
        // clear_color.float32[0] = 0.1f; // R
        // clear_color.float32[1] = 0.2f; // G
        // clear_color.float32[2] = 0.5f; // B
        // clear_color.float32[3] = 1.0f; // A

        // VkImageSubresourceRange clear_range{};
        // clear_range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        // clear_range.baseMipLevel = 0;
        // clear_range.levelCount = 1;
        // clear_range.baseArrayLayer = 0;
        // clear_range.layerCount = 1;

        // vkCmdClearColorImage(cmd_buffer, swapchain_image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, &clear_color, 1, &clear_range);

        // // Transfer destination -> present source
        // transitionImage(cmd_buffer, swapchain_image,
        //                 VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        //                 VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
        //                 VK_ACCESS_TRANSFER_WRITE_BIT,
        //                 0,
        //                 VK_PIPELINE_STAGE_TRANSFER_BIT,
        //                 VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT);

        app_gui.beginFrame();
        app_gui.showDemo();
        app_gui.endFrame(cmd_buffer, swapchain_image_index);

        if (vkEndCommandBuffer(cmd_buffer) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to end command buffer.");
        }

        // Submit
        // VkPipelineStageFlags wait_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        VkPipelineStageFlags wait_stage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        VkSubmitInfo submit_info{};
        submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submit_info.waitSemaphoreCount = 1;
        submit_info.pWaitSemaphores = &current_frame.image_available_semaphore;
        submit_info.pWaitDstStageMask = &wait_stage;
        submit_info.commandBufferCount = 1;
        submit_info.pCommandBuffers = &cmd_buffer;
        submit_info.signalSemaphoreCount = 1;
        submit_info.pSignalSemaphores = &app_swapchain.getRenderCompleteSemaphores()[swapchain_image_index];

        if (vkQueueSubmit(app_device.getGraphicsQueue(), 1, &submit_info, current_frame.render_fence) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to submit command buffer");
        }

        // Present
        VkSwapchainKHR swapchain = app_swapchain.getSwapchain();

        VkPresentInfoKHR present_info{};
        present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        present_info.waitSemaphoreCount = 1;
        present_info.pWaitSemaphores = &app_swapchain.getRenderCompleteSemaphores()[swapchain_image_index];
        present_info.swapchainCount = 1;
        present_info.pSwapchains = &swapchain;
        present_info.pImageIndices = &swapchain_image_index;

        if (vkQueuePresentKHR(app_device.getGraphicsQueue(), &present_info) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to present.");
        }

        ++frame_counter;
    }

    // Clean up
    app_device.deviceWaitIdle();
    app_gui.destroyGuiContext(app_device);
    app_frame_manager.destroyFrameData(app_device.getDevice());
    app_allocator.getGlobalDeletionQueue().flush();
    app_swapchain.destroySwapchain(app_device.getDevice());
    app_device.destroyDevice();
    app_window.destroySurface(app_instance.getInstance());
    app_instance.destroyInstance();

    return EXIT_SUCCESS;
}