#pragma once

#include "common_types.h"

class Instance
{
public:
    Instance();
    ~Instance();

    inline VkInstance getInstance() const { return instance; }
    inline VkDebugUtilsMessengerEXT getDebugMessenger() const { return debug_messenger; }
    inline vkb::Instance& getVkbInstance() { return vkb_instance; }

    void destroyInstance();

private:
    VkInstance instance;
    vkb::Instance vkb_instance;
    VkDebugUtilsMessengerEXT debug_messenger;
    bool is_instance_valid;

    void buildInstance();
};