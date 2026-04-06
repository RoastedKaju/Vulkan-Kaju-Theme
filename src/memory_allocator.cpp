#include "memory_allocator.h"

void MemoryAllocator::createAllocator(VkInstance instance, VkPhysicalDevice physical_device, VkDevice device)
{
    VmaAllocatorCreateInfo create_info{};
    create_info.physicalDevice = physical_device;
    create_info.device = device;
    create_info.instance = instance;
    create_info.flags = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT;

    vmaCreateAllocator(&create_info, &allocator);

    // Push the deletor
    global_deletion_queue.pushDeletor([&]()
                                      { vmaDestroyAllocator(allocator); });

    std::cout << "Created VMA memory allocator.\n";
}

void DeletionQueue::pushDeletor(std::function<void()> &&function)
{
    deletors.push_back(function);
}

void DeletionQueue::flush()
{
    for (auto it = deletors.rbegin(); it != deletors.rend(); ++it)
    {
        (*it)();
    }

    deletors.clear();
}
