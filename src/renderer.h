#pragma once

#include "common_types.h"

#include "image.h"

struct FrameData;
class FrameManager;
class Device;
class Swapchain;
class MemoryAllocator;
class KajuWindow;

class Renderer
{
public:
    Renderer() = default;
    ~Renderer() = default;

    inline uint32_t getCurrentFrameNumber() const { return frame_counter; }
    inline uint32_t getSwapchainImageIndex() const { return swapchain_image_index; }
    inline VkImage getSwapchainImage() const { return swapchain_image; }
    inline VkCommandBuffer getCommandBuffer() const { return cmd_buffer; }

    void createOffscreenRenderTarget(Device &device, MemoryAllocator &allocator, KajuWindow &window);
    void sync(FrameManager &frame_manager, Device &device, Swapchain &swapchain);
    void recordCommands();
    void beginRendering();
    void endRendering();
    void submit(Device &device, Swapchain &swapchain, FrameManager &frame_manager, KajuWindow &window);

    void prepareSwapchainImage();
    void finalizeSwapchainImage();

    void prepareOffscreenImage();
    void transitionOffscreenToShaderRead();

private:
    uint32_t frame_counter = 0;
    FrameData *current_frame;
    uint32_t swapchain_image_index;
    VkImage swapchain_image;
    VkCommandBuffer cmd_buffer;

    // Offscreen draw resources
    Image offscreen_render_target;
};