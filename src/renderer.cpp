#include "renderer.h"

#include "frame_manager.h"
#include "device.h"
#include "swapchain.h"

void Renderer::sync(FrameManager &frame_manager, Device &device, Swapchain &swapchain)
{
    current_frame = &frame_manager.getFrame(frame_counter);

    if (vkWaitForFences(device.getDevice(), 1, &current_frame->render_fence, VK_TRUE, 1000000000) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to wait for render fence.");
    }
    // Delete all resources for current frame
    current_frame->frame_deletion_queue.flush();
    // Reset fence
    if (vkResetFences(device.getDevice(), 1, &current_frame->render_fence) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to reset render fence.");
    }
    // Request image from swapchain
    VkResult acquire_result = vkAcquireNextImageKHR(device.getDevice(), swapchain.getSwapchain(), 1000000000, current_frame->image_available_semaphore, VK_NULL_HANDLE, &swapchain_image_index);
    if (acquire_result != VK_SUCCESS && acquire_result != VK_SUBOPTIMAL_KHR)
    {
        throw std::runtime_error("Failed to acquire swapchain image.");
    }
    swapchain_image = swapchain.getImages()[swapchain_image_index];
}

void Renderer::recordCommands()
{
    // Record commands
    cmd_buffer = current_frame->command_buffer;
    // Reset the command buffer before recording commands into it
    if (vkResetCommandBuffer(cmd_buffer, 0) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to reset command buffer.");
    }
}

void Renderer::beginRendering()
{
    // Begin rendering commands
    VkCommandBufferBeginInfo begin_info{};
    begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    if (vkBeginCommandBuffer(cmd_buffer, &begin_info) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to begin command buffer.");
    }
}

void Renderer::endRendering()
{
    if (vkEndCommandBuffer(cmd_buffer) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to end command buffer.");
    }
}

void Renderer::submit(Device &device, Swapchain &swapchain, FrameManager &frame_manager)
{
    // Submit
    // VkPipelineStageFlags wait_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    VkPipelineStageFlags wait_stage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    VkSubmitInfo submit_info{};
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit_info.waitSemaphoreCount = 1;
    submit_info.pWaitSemaphores = &current_frame->image_available_semaphore;
    submit_info.pWaitDstStageMask = &wait_stage;
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &cmd_buffer;
    submit_info.signalSemaphoreCount = 1;
    submit_info.pSignalSemaphores = &swapchain.getRenderCompleteSemaphores()[swapchain_image_index];

    if (vkQueueSubmit(device.getGraphicsQueue(), 1, &submit_info, current_frame->render_fence) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to submit command buffer");
    }

    // Present
    VkSwapchainKHR present_swapchain = swapchain.getSwapchain();

    VkPresentInfoKHR present_info{};
    present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    present_info.waitSemaphoreCount = 1;
    present_info.pWaitSemaphores = &swapchain.getRenderCompleteSemaphores()[swapchain_image_index];
    present_info.swapchainCount = 1;
    present_info.pSwapchains = &present_swapchain;
    present_info.pImageIndices = &swapchain_image_index;

    if (vkQueuePresentKHR(device.getGraphicsQueue(), &present_info) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to present.");
    }

    frame_counter = (frame_counter + 1) % frame_manager.getOverlapFrameCount();
}
