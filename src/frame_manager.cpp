#include "frame_manager.h"
#include "device.h"

FrameManager::FrameManager(uint8_t overlap_count)
    : overlap_frame_count{overlap_count}
{
    frames.resize(overlap_count, FrameData{});
}

void FrameManager::createFrameData(Device &device)
{
    // Pool info
    VkCommandPoolCreateInfo cmd_pool_create_info{};
    cmd_pool_create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    cmd_pool_create_info.pNext = nullptr;
    cmd_pool_create_info.queueFamilyIndex = device.getGraphicsQueueFamily();
    cmd_pool_create_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

    for (int i = 0; i < overlap_frame_count; ++i)
    {
        if (vkCreateCommandPool(device.getDevice(), &cmd_pool_create_info, nullptr, &frames[i].command_pool) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to create command pool.");
        }

        // Buffer info
        VkCommandBufferAllocateInfo cmd_buffer_create_info = {};
        cmd_buffer_create_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        cmd_buffer_create_info.pNext = nullptr;
        cmd_buffer_create_info.commandPool = frames[i].command_pool;
        cmd_buffer_create_info.commandBufferCount = 1;
        cmd_buffer_create_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

        if (vkAllocateCommandBuffers(device.getDevice(), &cmd_buffer_create_info, &frames[i].command_buffer) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to allocate command buffer.");
        }
    }

    // Initialize per-frame semaphores and fences
    createSyncStructure(device.getDevice());
}

void FrameManager::createSyncStructure(VkDevice device)
{
    VkFenceCreateInfo fence_create_info{};
    fence_create_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fence_create_info.pNext = nullptr;
    fence_create_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    VkSemaphoreCreateInfo semaphore_create_info{};
    semaphore_create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    semaphore_create_info.pNext = nullptr;
    semaphore_create_info.flags = 0;

    for (size_t i = 0; i < overlap_frame_count; ++i)
    {
        if (vkCreateFence(device, &fence_create_info, nullptr, &frames[i].render_fence) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to create render fence.");
        }

        if (vkCreateSemaphore(device, &semaphore_create_info, nullptr, &frames[i].image_available_semaphore) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to create acquire semaphore.");
        }
    }
}

void FrameManager::destroyFrameData(VkDevice device)
{
    for (int i = 0; i < overlap_frame_count; ++i)
    {
        vkDestroyCommandPool(device, frames[i].command_pool, nullptr);

        // destroy sync objects
        vkDestroyFence(device, frames[i].render_fence, nullptr);
        vkDestroySemaphore(device, frames[i].image_available_semaphore, nullptr);

        frames[i].frame_deletion_queue.flush();
    }

    frames.clear();
}
