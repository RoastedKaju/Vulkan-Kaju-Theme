#define VOLK_IMPLEMENTATION
#define VMA_IMPLEMENTATION
#include <context.hpp>
#include <iostream>
#include <GLFW/glfw3.h>

static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT severity, VkDebugUtilsMessageTypeFlagsEXT type, const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData, void *pUserData)
{
    if (severity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
    {
        std::cerr << "Validation Layer: " << pCallbackData->pMessage << std::endl;
    }

    return VK_FALSE;
}

void Context::init(GLFWwindow *inWindow)
{
    pWindow = inWindow;

    createInstance();
    createSurface();
    findPhysicalDevice();
    findGraphicsQueue();
    createDevice();
    initializeVMA();
}

void Context::createInstance()
{
    if (volkInitialize() != VK_SUCCESS)
    {
        throw std::runtime_error("Error initializing Volk");
    }

    VkApplicationInfo appInfo{
        .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .pApplicationName = "Vulkan Application",
        .apiVersion = cVulkanVersion};

    uint32_t instExtCount = 0;
    const char **extensions = glfwGetRequiredInstanceExtensions(&instExtCount);
    std::vector<const char *> requestedExtensions{VK_EXT_DEBUG_UTILS_EXTENSION_NAME};
    for (int i = 0; i < instExtCount; ++i)
    {
        requestedExtensions.push_back(extensions[i]);
    }

    // enable validation layer
    std::vector<const char *> requestedLayers{"VK_LAYER_KHRONOS_validation"};

    VkDebugUtilsMessengerCreateInfoEXT debugInfo{
        .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
        .messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
                           VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                           VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
        .messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                       VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
        .pfnUserCallback = debugCallback};

    VkInstanceCreateInfo instCreateInfo{
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pNext = &debugInfo,
        .pApplicationInfo = &appInfo,
        .enabledLayerCount = (uint32_t)requestedLayers.size(),
        .ppEnabledLayerNames = requestedLayers.data(),
        .enabledExtensionCount = (uint32_t)requestedExtensions.size(),
        .ppEnabledExtensionNames = requestedExtensions.data()};

    if (vkCreateInstance(&instCreateInfo, nullptr, &mInstance) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create instance");
    }
    volkLoadInstance(mInstance);

    std::cout << "Instance created.\n";
}

void Context::createSurface()
{
    if (glfwCreateWindowSurface(mInstance, pWindow, nullptr, &mSurface) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create window surface");
    }

    std::cout << "Surface created.\n";
}

void Context::findPhysicalDevice()
{
    // enumerate all physical devices
    uint32_t physDeviceCount = 0;
    vkEnumeratePhysicalDevices(mInstance, &physDeviceCount, nullptr);
    std::vector<VkPhysicalDevice> devices(physDeviceCount);
    vkEnumeratePhysicalDevices(mInstance, &physDeviceCount, devices.data());

    if (physDeviceCount)
    {
        mPhysicalDevice = devices[0];
        for (auto &dev : devices)
        {
            VkPhysicalDeviceProperties props{};
            vkGetPhysicalDeviceProperties(dev, &props);
            if (props.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
            {
                mPhysicalDevice = dev;
                std::cout << "Physical device: " << props.deviceName << ".\n";
                return;
            }
        }
    }

    throw std::runtime_error("Failed to find proper physical device");
}

void Context::findGraphicsQueue()
{
    // grab all of the queue families
    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties2(mPhysicalDevice, &queueFamilyCount, nullptr);
    std::vector<VkQueueFamilyProperties2> queueFamilyProps(queueFamilyCount, {.sType = VK_STRUCTURE_TYPE_QUEUE_FAMILY_PROPERTIES_2});
    vkGetPhysicalDeviceQueueFamilyProperties2(mPhysicalDevice, &queueFamilyCount, queueFamilyProps.data());

    for (int i = 0; i < queueFamilyProps.size(); ++i)
    {
        VkBool32 hasPresentSupport = VK_FALSE;
        vkGetPhysicalDeviceSurfaceSupportKHR(mPhysicalDevice, i, mSurface, &hasPresentSupport);

        const auto &props = queueFamilyProps[i];
        if (props.queueFamilyProperties.queueFlags & VK_QUEUE_GRAPHICS_BIT && hasPresentSupport)
        {
            mGraphicsQueueFamily = i;
            std::cout << "Found graphics queue at index: " << i << ".\n";
            return;
        }
    }

    throw std::runtime_error("Failed to find graphics queue family index");
}

void Context::createDevice()
{
    // query supported features
    {
        VkPhysicalDeviceVulkan14Features features14{
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_4_FEATURES,
            .pNext = nullptr};
        VkPhysicalDeviceVulkan13Features features13{
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES,
            .pNext = &features14};
        VkPhysicalDeviceVulkan12Features features12{
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES,
            .pNext = &features13};
        VkPhysicalDeviceFeatures2 supportedFeatures{
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2,
            .pNext = &features12};

        vkGetPhysicalDeviceFeatures2(mPhysicalDevice, &supportedFeatures);

        if (!features13.dynamicRendering || !features13.synchronization2 || !features12.timelineSemaphore)
        {
            throw std::runtime_error("Physical device does not meet the features requirement");
        }
    }

    // produce a separate features struct chain for device creation
    VkPhysicalDeviceVulkan14Features features14{
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_4_FEATURES,
        .pNext = nullptr};
    VkPhysicalDeviceVulkan13Features features13{
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES,
        .pNext = &features14,
        .synchronization2 = VK_TRUE,
        .dynamicRendering = VK_TRUE};
    VkPhysicalDeviceVulkan12Features features12{
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES,
        .pNext = &features13,
        .timelineSemaphore = VK_TRUE};
    VkPhysicalDeviceFeatures2 supportedFeatures{
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2,
        .pNext = &features12};

    // request the queues we will be using
    std::vector<float> queuePriorities{1.0f};
    VkDeviceQueueCreateInfo gfxQueueInfo{
        .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
        .queueFamilyIndex = mGraphicsQueueFamily,
        .queueCount = 1,
        .pQueuePriorities = queuePriorities.data()};

    const std::vector<const char *> deviceExtensions{VK_KHR_SWAPCHAIN_EXTENSION_NAME};

    VkDeviceCreateInfo devCreateInfo{
        .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        .pNext = &supportedFeatures,
        .queueCreateInfoCount = 1,
        .pQueueCreateInfos = &gfxQueueInfo,
        .enabledExtensionCount = (uint32_t)deviceExtensions.size(),
        .ppEnabledExtensionNames = deviceExtensions.data(),
        .pEnabledFeatures = nullptr};

    if (vkCreateDevice(mPhysicalDevice, &devCreateInfo, nullptr, &mDevice) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create device");
    }
    volkLoadDevice(mDevice);

    // grab the queue object
    vkGetDeviceQueue(mDevice, mGraphicsQueueFamily, 0, &mGraphicsQueue);
    if (!mGraphicsQueue)
    {
        throw std::runtime_error("Couldn't get the graphics queue");
    }

    std::cout << "Logical device created.\n";
}

void Context::initializeVMA()
{
    VmaVulkanFunctions vmaFuncInfo{};
    VmaAllocatorCreateInfo vmaAllocInfo{
        .flags = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT,
        .physicalDevice = mPhysicalDevice,
        .device = mDevice,
        .pVulkanFunctions = &vmaFuncInfo,
        .instance = mInstance,
        .vulkanApiVersion = cVulkanVersion};

    // VMA can import directly from volk
    vmaImportVulkanFunctionsFromVolk(&vmaAllocInfo, &vmaFuncInfo);

    if (vmaCreateAllocator(&vmaAllocInfo, &mAllocator) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create VMA");
    }

    std::cout << "Initialized VMA.\n";
}

void Context::shutdown()
{
    if (mDevice)
    {
        vkDestroyDevice(mDevice, nullptr);
        mDevice = VK_NULL_HANDLE;
    }
    // VMA
    if (mAllocator)
    {
        vmaDestroyAllocator(mAllocator);
        mAllocator = VK_NULL_HANDLE;
    }
    if (mSurface)
    {
        vkDestroySurfaceKHR(mInstance, mSurface, nullptr);
        mSurface = VK_NULL_HANDLE;
    }
    if (mInstance)
    {
        vkDestroyInstance(mInstance, nullptr);
        mInstance = VK_NULL_HANDLE;
    }
    volkFinalize();
}