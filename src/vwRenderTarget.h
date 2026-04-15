#pragma once

#include "vwCommon.h"
#include "vwUtils.h"
#include "vwImage.h"

namespace vw
{
    class Allocator;
    class Instance;

    class RenderTarget
    {
    public:
        void createRenderTarget(VkExtent2D extent, Allocator &allocator, Instance &instance);
        void destroyRenderTarget(Instance &instance, Allocator &allocator);
        void draw();

        inline VkImageView getDrawImageView() const { return drawImage.view; }

    private:
        AllocatedImage drawImage;
        VkExtent2D drawExtent;
    };
}