#pragma once

#include "common_types.h"
#include "memory_allocator.h"

class Device;

struct FrameData
{
    VkCommandPool command_pool;
    VkCommandBuffer command_buffer;
    VkSemaphore image_available_semaphore;
    VkFence render_fence;
    DeletionQueue frame_deletion_queue;
};

class FrameManager
{
public:
    FrameManager(uint8_t overlap_count = 2);
    ~FrameManager() = default;

    inline FrameData &getFrame(uint32_t frame_number) { return frames[frame_number % overlap_frame_count]; }

    void createFrameData(Device &device);
    void destroyFrameData(VkDevice device);

private:
    uint8_t overlap_frame_count;
    std::vector<FrameData> frames;

    void createSyncStructure(VkDevice device);
};