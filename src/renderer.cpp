#include "renderer.h"

#include "frame_manager.h"
#include "device.h"
#include "swapchain.h"
#include "kaju_window.h"

void Renderer::createOffscreenRenderTarget(Device &device, MemoryAllocator &allocator, KajuWindow &window)
{
    // Get initial window extent
    int width = 0, height = 0;
    glfwGetFramebufferSize(window.getWindow(), &width, &height);

    offscreen_render_target.format = VK_FORMAT_R16G16B16A16_SFLOAT;
    offscreen_render_target.extent = VkExtent3D(width, height, 1);

    VkImageUsageFlags draw_image_usages{};
    draw_image_usages |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    draw_image_usages |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    draw_image_usages |= VK_IMAGE_USAGE_STORAGE_BIT;
    draw_image_usages |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    draw_image_usages |= VK_IMAGE_USAGE_SAMPLED_BIT;
    VkImageCreateInfo image_create_info = {};
    image_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    image_create_info.pNext = nullptr;
    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.format = offscreen_render_target.format;
    image_create_info.extent = offscreen_render_target.extent;
    image_create_info.mipLevels = 1;
    image_create_info.arrayLayers = 1;
    image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_create_info.usage = draw_image_usages;

    VmaAllocationCreateInfo allocation_create_info = {};
    allocation_create_info.usage = VMA_MEMORY_USAGE_GPU_ONLY;
    allocation_create_info.requiredFlags = VkMemoryPropertyFlags(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    // Use allocator to create image
    vmaCreateImage(allocator.getAllocator(), &image_create_info, &allocation_create_info, &offscreen_render_target.image, &offscreen_render_target.allocation, nullptr);

    // Create off screen render target image view
    // build a image-view for the depth image to use for rendering
    VkImageViewCreateInfo image_view_create_info = {};
    image_view_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    image_view_create_info.pNext = nullptr;

    image_view_create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
    image_view_create_info.image = offscreen_render_target.image;
    image_view_create_info.format = offscreen_render_target.format;
    image_view_create_info.subresourceRange.baseMipLevel = 0;
    image_view_create_info.subresourceRange.levelCount = 1;
    image_view_create_info.subresourceRange.baseArrayLayer = 0;
    image_view_create_info.subresourceRange.layerCount = 1;
    image_view_create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

    if (vkCreateImageView(device.getDevice(), &image_view_create_info, nullptr, &offscreen_render_target.image_view) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create draw image view.");
    }

    // Add the image to deletion queue
    allocator.getGlobalDeletionQueue().pushDeletor([&]()
                                                   {
        vkDestroyImageView(device.getDevice(), offscreen_render_target.image_view, nullptr);
        vmaDestroyImage(allocator.getAllocator(), offscreen_render_target.image, offscreen_render_target.allocation); });
}

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

void Renderer::submit(Device &device, Swapchain &swapchain, FrameManager &frame_manager, KajuWindow &window)
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

    VkResult present_result = vkQueuePresentKHR(device.getGraphicsQueue(), &present_info);

    if (present_result == VK_ERROR_OUT_OF_DATE_KHR || present_result == VK_SUBOPTIMAL_KHR || window.frameBufferResized)
    {
        device.deviceWaitIdle();
        window.frameBufferResized = false;
        // recreate swapchain
        swapchain.createSwapchain(device, window);
        return;
    }

    if (present_result != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to present.");
    }

    frame_counter = (frame_counter + 1) % frame_manager.getOverlapFrameCount();
}

void Renderer::prepareSwapchainImage()
{
    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    barrier.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = swapchain_image;
    barrier.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};
    barrier.srcAccessMask = 0;
    barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    vkCmdPipelineBarrier(cmd_buffer, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);
}

void Renderer::finalizeSwapchainImage()
{
    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    barrier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = swapchain_image;
    barrier.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};
    barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    barrier.dstAccessMask = 0;

    vkCmdPipelineBarrier(cmd_buffer, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);
}

void Renderer::prepareOffscreenImage()
{
    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    barrier.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = offscreen_render_target.image;
    barrier.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};
    barrier.srcAccessMask = 0;
    barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    vkCmdPipelineBarrier(cmd_buffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);
}

void Renderer::transitionOffscreenToShaderRead()
{
    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = offscreen_render_target.image;
    barrier.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};
    barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

    vkCmdPipelineBarrier(cmd_buffer, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);
}
