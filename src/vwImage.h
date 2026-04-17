#pragma once

#include "vwCommon.h"

namespace vw
{
    struct AllocatedImage
    {
        VkImage image;
        VkImageView view;
        VmaAllocation allocation;
        VkExtent3D extent;
        VkFormat format;
    };
}