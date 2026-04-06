#pragma once

#include "common_types.h"

class Instance;
class Device;

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

    void createAllocator(Instance &instance, Device &device);
    void destroyAllocator();

    inline DeletionQueue &getGlobalDeletionQueue() { return global_deletion_queue; }

private:
    VmaAllocator allocator;
    DeletionQueue global_deletion_queue;
};