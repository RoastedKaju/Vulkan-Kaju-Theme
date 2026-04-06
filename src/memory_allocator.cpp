#include "memory_allocator.h"

#include "instance.h"
#include "device.h"

void MemoryAllocator::createAllocator(Instance &instance, Device &device)
{
    VmaAllocatorCreateInfo create_info{};
    create_info.physicalDevice = device.getPhysicalDevice();
    create_info.device = device.getDevice();
    create_info.instance = instance.getInstance();
    create_info.flags = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT;

    vmaCreateAllocator(&create_info, &allocator);

    // Push the deletor
    global_deletion_queue.pushDeletor([&]()
                                      { vmaDestroyAllocator(allocator); });

    std::cout << "Created VMA memory allocator.\n";
}

void MemoryAllocator::destroyAllocator()
{
    global_deletion_queue.flush();
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
