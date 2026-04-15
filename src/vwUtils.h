#pragma once

#include "vwCommon.h"

namespace vw::utils
{
    inline VkSemaphoreCreateInfo generateSemaphoreCreateInfo()
    {
        VkSemaphoreCreateInfo info{};
        info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
        info.pNext = nullptr;
        info.flags = 0;
        return info;
    }
}