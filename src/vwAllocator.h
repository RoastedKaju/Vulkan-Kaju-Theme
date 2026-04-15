#pragma once

#include "vwCommon.h"

namespace vw
{
    class Instance;

    struct DeletionQueue
    {
        std::deque<std::function<void()>> deletors;

        void pushDeletor(std::function<void()> &&function);
        void flush();
    };

    class Allocator
    {
    public:
        Allocator() = default;
        ~Allocator() = default;

        void createAllocator(Instance &instance);
        void destroyAllocator();

        inline DeletionQueue &getGlobalDeletionQueue() { return globalDeletionQueue; }
        inline VmaAllocator getAllocator() const { return allocator; }

    private:
        VmaAllocator allocator;
        DeletionQueue globalDeletionQueue;
    };
}