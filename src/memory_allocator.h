#pragma once

#include "common_types.h"

struct DeletionQueue
{
    std::deque<std::function<void()>> deletors;

    void pushDeletor(std::function<void()> &&function);
    void flush();
};

class MemoryAllocator
{
public:
    MemoryAllocator() = default;
    ~MemoryAllocator() = default;

    void createAllocator(VkInstance instance, VkPhysicalDevice physical_device, VkDevice device);

    inline DeletionQueue &getGlobalDeletionQueue() { return global_deletion_queue; }

private:
    VmaAllocator allocator;
    DeletionQueue global_deletion_queue;
};