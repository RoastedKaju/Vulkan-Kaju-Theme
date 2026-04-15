#include "vwAllocator.h"
#include "vwInstance.h"

void vw::DeletionQueue::pushDeletor(std::function<void()> &&function)
{
    deletors.push_back(function);
}

void vw::DeletionQueue::flush()
{
    for (auto it = deletors.rbegin(); it != deletors.rend(); ++it)
    {
        (*it)();
    }

    deletors.clear();
}

void vw::Allocator::createAllocator(Instance &instance)
{
    VmaAllocatorCreateInfo create_info{};
    create_info.physicalDevice = instance.getPhysicalDevice();
    create_info.device = instance.getDevice();
    create_info.instance = instance.getInstance();
    create_info.flags = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT;

    vmaCreateAllocator(&create_info, &allocator);

    // Push the deletor
    globalDeletionQueue.pushDeletor([&]()
                                    { vmaDestroyAllocator(allocator); });

    std::cout << "Created VMA.\n";
}

void vw::Allocator::destroyAllocator()
{
    globalDeletionQueue.flush();
    std::cout << "Destroyed VMA.\n";
}
