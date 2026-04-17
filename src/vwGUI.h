#pragma once

#include "vwCommon.h"
#include "vwUtils.h"

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_vulkan.h>
#include <imgui_internal.h>

namespace vw
{
    class Swapchain;

    class GUI
    {
    public:
        void createGUIContext(GLFWwindow *window, const utils::Context &context);
        void destroyGUIContext(VkDevice device);

        inline VkExtent2D getViewportSize() const { return VkExtent2D{(uint32_t)viewportSize.x, (uint32_t)viewportSize.y}; }

        void beginFrame();
        void endFrame(VkCommandBuffer cmd, Swapchain& swapchain, uint32_t swapchainImageIndex);
    private:
        VkDescriptorPool descriptorPool;
        VkSampler viewportSampler;
        ImTextureID viewportTexture;
        ImVec2 viewportSize;
    };
}