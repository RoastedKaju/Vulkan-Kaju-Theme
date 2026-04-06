#include "instance.h"

Instance::Instance()
    : instance(VK_NULL_HANDLE),
      debug_messenger(VK_NULL_HANDLE),
      is_instance_valid(false)
{
    std::cout << "Creating VK Instance\n";

    buildInstance();
}

Instance::~Instance()
{
    std::cout << "Destroying VK Instance\n";
}

void Instance::destroyInstance()
{
    vkb::destroy_debug_utils_messenger(instance, debug_messenger);
    vkDestroyInstance(instance, nullptr);
}

void Instance::buildInstance()
{
    vkb::InstanceBuilder builder;

    auto vkb_instance_result = builder.set_app_name("Kaju Theme")
                                   .request_validation_layers(true)
                                   .use_default_debug_messenger()
                                   .require_api_version(1, 3, 0)
                                   .build();

    vkb_instance = vkb_instance_result.value();
    instance = vkb_instance.instance;
    debug_messenger = vkb_instance.debug_messenger;
}
