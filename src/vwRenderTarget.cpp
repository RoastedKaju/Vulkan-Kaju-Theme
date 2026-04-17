#include "vwRenderTarget.h"
#include "vwAllocator.h"
#include "vwInstance.h"

void vw::RenderTarget::createRenderTarget(VkExtent2D extent, Allocator &allocator, Instance &instance)
{
    // First, destroy old resources if they exist
    destroyRenderTarget(instance, allocator);

    VkExtent3D drawImageExtent = {extent.width, extent.height, 1};

    drawImage.format = VK_FORMAT_R16G16B16A16_SFLOAT;
    drawImage.extent = drawImageExtent;

    VkImageUsageFlags drawImageUsages{};
    drawImageUsages |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    drawImageUsages |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    drawImageUsages |= VK_IMAGE_USAGE_STORAGE_BIT;
    drawImageUsages |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    drawImageUsages |= VK_IMAGE_USAGE_SAMPLED_BIT;

    VkImageCreateInfo drawImageInfo = utils::imageCreateInfo(drawImage.format, drawImageUsages, drawImageExtent);

    VmaAllocationCreateInfo drawImageAllocInfo = {};
    drawImageAllocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
    drawImageAllocInfo.requiredFlags = VkMemoryPropertyFlags(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    // allocate image
    vmaCreateImage(allocator.getAllocator(), &drawImageInfo, &drawImageAllocInfo, &drawImage.image, &drawImage.allocation, nullptr);
    // create image view
    VkImageViewCreateInfo drawImageViewInfo = utils::imageViewCreateInfo(drawImage.format, drawImage.image, VK_IMAGE_ASPECT_COLOR_BIT);

    VW_CHECK(vkCreateImageView(instance.getDevice(), &drawImageViewInfo, nullptr, &drawImage.view));

    // add to deletion queue
    // auto drawImageDeletionFunc = [&]()
    // {
    //     vkDestroyImageView(instance.getDevice(), drawImage.view, nullptr);
    //     vmaDestroyImage(allocator.getAllocator(), drawImage.image, drawImage.allocation);
    // };

    // allocator.getGlobalDeletionQueue().pushDeletor(drawImageDeletionFunc);

    std::cout << "Created render target image.\n";
}

void vw::RenderTarget::destroyRenderTarget(Instance &instance, Allocator &allocator)
{
    if (drawImage.view != VK_NULL_HANDLE)
    {
        vkDestroyImageView(instance.getDevice(), drawImage.view, nullptr);
        drawImage.view = VK_NULL_HANDLE;
    }
    if (drawImage.image != VK_NULL_HANDLE)
    {
        vmaDestroyImage(allocator.getAllocator(), drawImage.image, drawImage.allocation);
        drawImage.image = VK_NULL_HANDLE;
        drawImage.allocation = VK_NULL_HANDLE;
        std::cout << "Destroyed render target image.\n";
    }
}

void vw::RenderTarget::draw()
{
}
