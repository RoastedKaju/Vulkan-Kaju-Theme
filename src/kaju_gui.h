#pragma once

#include "common_types.h"

class Device;
class Swapchain;
class KajuWindow;
class Instance;

class KajuGui
{
public:
    KajuGui() = default;
    ~KajuGui() = default;

    void createGuiContext(Device &device, Swapchain &swapchain, KajuWindow &window, Instance &instance);
    void beginFrame();
    void endFrame(VkCommandBuffer command_buffer, Swapchain& swapchain, uint32_t swapchain_image_index);
    void destroyGuiContext(Device &device);

    void showDemo();

private:
    VkDescriptorPool descriptor_pool;
};