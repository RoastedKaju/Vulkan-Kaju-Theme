#define VOLK_IMPLEMENTATION
#define VMA_IMPLEMENTATION
#include <context.hpp>
#include <iostream>
#include <GLFW/glfw3.h>
#include <utils.hpp>

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

    int width, height;
    glfwGetFramebufferSize(inWindow, &width, &height);

    createInstance();
    createSurface();
    findPhysicalDevice();
    findGraphicsQueue();
    createDevice();
    initializeVMA();
    createSwapchain(width, height);
    createShaders();
    createGraphicsPipeline();
    createSyncResources();
    createCommandBuffers();
}

VkShaderModule Context::createShaderModule(const std::string &fileName, shaderc_shader_kind kind) const
{
    // read shader file from disk
    const std::string shaderPath = "../../resources/shaders/" + fileName;
    const std::string src = readTextFile(shaderPath);
    if (src.empty())
    {
        throw std::runtime_error("Specified shader file doesn't exist " + shaderPath);
    }

    // compile shader to SPIR-V
    std::cout << "Compiling shader: " << shaderPath << std::endl;
    shaderc::Compiler compiler;
    shaderc::CompileOptions opts;
    opts.SetTargetEnvironment(shaderc_target_env_vulkan, shaderc_env_version_vulkan_1_4);
    opts.SetTargetSpirv(shaderc_spirv_version_1_6);
    opts.SetOptimizationLevel(shaderc_optimization_level_performance);
    shaderc::CompilationResult result = compiler.CompileGlslToSpv(src, kind, fileName.c_str(), opts);

    if (result.GetCompilationStatus() != shaderc_compilation_status_success)
    {
        std::cerr << "Shader compilation error: " << result.GetErrorMessage() << std::endl;
    }

    const size_t shaderSize = (result.cend() - result.cbegin()) * sizeof(uint32_t);
    // pass SPIR-V to vulkan and create shader module
    VkShaderModuleCreateInfo moduleCreateinfo{
        .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        .codeSize = shaderSize,
        .pCode = result.cbegin()};
    VkShaderModule shaderModule = nullptr;
    if (vkCreateShaderModule(mDevice, &moduleCreateinfo, nullptr, &shaderModule) != VK_SUCCESS)
    {
        throw std::runtime_error("Error creating shader module");
    }

    return shaderModule;
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
        bool foundPhysDevice = false;
        for (auto &dev : devices)
        {
            VkPhysicalDeviceProperties props{};
            vkGetPhysicalDeviceProperties(dev, &props);
            if (props.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
            {
                mPhysicalDevice = dev;
                foundPhysDevice = true;
                std::cout << "Physical device: " << props.deviceName << ".\n";
                break;
            }
        }

        if (!foundPhysDevice)
        {
            throw std::runtime_error("Failed to find proper physical device");
        }
    }

    // ensure the desired swapchain format is supported
    uint32_t formatCount = 0;
    vkGetPhysicalDeviceSurfaceFormatsKHR(mPhysicalDevice, mSurface, &formatCount, nullptr);
    std::vector<VkSurfaceFormatKHR> surfaceFormats(formatCount);
    vkGetPhysicalDeviceSurfaceFormatsKHR(mPhysicalDevice, mSurface, &formatCount, surfaceFormats.data());

    bool formatSupported = false;
    for (const VkSurfaceFormatKHR &surfFormat : surfaceFormats)
    {
        if (surfFormat.format == cSwapchainFormat)
        {
            formatSupported = true;
            break;
        }
    }

    if (!formatSupported)
    {
        throw std::runtime_error("Requested swapchain format is not supported by surface");
    }
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

void Context::createSwapchain(uint32_t inWidth, uint32_t inHeight)
{
    mSwapchainWidth = inWidth;
    mSwapchainHeight = inHeight;

    // ensure we request an appropriate number of images
    VkSurfaceCapabilitiesKHR surfaceCaps{};
    if (vkGetPhysicalDeviceSurfaceCapabilitiesKHR(mPhysicalDevice, mSurface, &surfaceCaps) != VK_SUCCESS)
    {
        throw std::runtime_error("Couldn't get surface capabilities");
    }

    uint32_t requestedImageCount = std::max(2u, surfaceCaps.minImageCount);
    if (surfaceCaps.maxImageCount > 0)
    {
        requestedImageCount = std::min(requestedImageCount, surfaceCaps.maxImageCount);
    }

    VkExtent2D extent;
    if (surfaceCaps.currentExtent.width != UINT32_MAX)
    {
        extent = surfaceCaps.currentExtent;
    }
    else
    {
        extent.width = std::clamp(inWidth, surfaceCaps.minImageExtent.width, surfaceCaps.maxImageExtent.width);
        extent.height = std::clamp(inHeight, surfaceCaps.minImageExtent.height, surfaceCaps.maxImageExtent.height);
    }

    VkSwapchainCreateInfoKHR swapchainCreateInfo{
        .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        .surface = mSurface,
        .minImageCount = requestedImageCount,
        .imageFormat = cSwapchainFormat,
        .imageColorSpace = VK_COLORSPACE_SRGB_NONLINEAR_KHR,
        .imageExtent = extent,
        .imageArrayLayers = 1,
        .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        .imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .preTransform = surfaceCaps.currentTransform,
        .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
        .presentMode = VK_PRESENT_MODE_FIFO_KHR,
        .clipped = VK_TRUE};

    if (vkCreateSwapchainKHR(mDevice, &swapchainCreateInfo, nullptr, &mSwapchain) != VK_SUCCESS)
    {
        throw std::runtime_error("Error creating swapchain");
    }

    // ask for swapchain images
    uint32_t imageCount = 0;
    vkGetSwapchainImagesKHR(mDevice, mSwapchain, &imageCount, nullptr);
    mSwapchainImages.resize(imageCount);
    vkGetSwapchainImagesKHR(mDevice, mSwapchain, &imageCount, mSwapchainImages.data());
    mSwapchainImageViews.resize(imageCount);

    // create swapchain image views
    for (size_t i = 0; i < mSwapchainImages.size(); ++i)
    {
        VkImageViewCreateInfo imgViewInfo{
            .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            .image = mSwapchainImages[i],
            .viewType = VK_IMAGE_VIEW_TYPE_2D,
            .format = cSwapchainFormat,
            .subresourceRange{
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                .baseMipLevel = 0,
                .levelCount = 1,
                .baseArrayLayer = 0,
                .layerCount = 1 // end sub-resource
            }};

        if (vkCreateImageView(mDevice, &imgViewInfo, nullptr, &mSwapchainImageViews[i]) != VK_SUCCESS)
        {
            throw std::runtime_error("Error creating swapchain image view");
        }
    }

    // semaphores used to signal render completion
    mRenderCompleteSemaphores.resize(mSwapchainImages.size());
    for (VkSemaphore &semaphore : mRenderCompleteSemaphores)
    {
        VkSemaphoreCreateInfo semaphoreInfo{.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};
        if (vkCreateSemaphore(mDevice, &semaphoreInfo, nullptr, &semaphore) != VK_SUCCESS)
        {
            throw std::runtime_error("Error creating render-complete semaphore");
        }
    }

    // create depth image
    VkImageCreateInfo depthCreateInfo{
        .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
        .imageType = VK_IMAGE_TYPE_2D,
        .format = cDepthFormat,
        .extent{.width = mSwapchainWidth, .height = mSwapchainHeight, .depth = 1},
        .mipLevels = 1,
        .arrayLayers = 1,
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .tiling = VK_IMAGE_TILING_OPTIMAL,
        .usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED};

    VmaAllocationCreateInfo allocInfo{
        .flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT,
        .usage = VMA_MEMORY_USAGE_AUTO};

    if (vmaCreateImage(mAllocator, &depthCreateInfo, &allocInfo, &mDepthImage, &mDepthImageAllocation, nullptr) != VK_SUCCESS)
    {
        throw std::runtime_error("Error allocating depth image");
    }

    VkImageViewCreateInfo depthImgViewInfo{
        .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        .image = mDepthImage,
        .viewType = VK_IMAGE_VIEW_TYPE_2D,
        .format = cDepthFormat,
        .subresourceRange{
            .aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT,
            .levelCount = 1,
            .layerCount = 1}};
    if (vkCreateImageView(mDevice, &depthImgViewInfo, nullptr, &mDepthImageView) != VK_SUCCESS)
    {
        throw std::runtime_error("Error creating depth image view");
    }

    std::cout << "Created swapchain with " << mSwapchainImages.size() << " images.\n";
    std::cout << "Depth image created.\n";
}

void Context::createGraphicsPipeline()
{
    // need to define a pipeline layout
    VkPipelineLayoutCreateInfo pipelineLayoutInfo{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        .setLayoutCount = 0,
        .pushConstantRangeCount = 0};
    if (vkCreatePipelineLayout(mDevice, &pipelineLayoutInfo, nullptr, &mPipelineLayout) != VK_SUCCESS)
    {
        throw std::runtime_error("Unable to create pipeline layout");
    }

    // configure shader stages struct
    const char *entryPoint = "main";
    std::vector<VkPipelineShaderStageCreateInfo> shaderStages{
        {.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
         .stage = VK_SHADER_STAGE_VERTEX_BIT,
         .module = mVertShader,
         .pName = entryPoint},
        {.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
         .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
         .module = mFragShader,
         .pName = entryPoint}};

    // vertex pulling, don't define vertex input details
    VkPipelineVertexInputStateCreateInfo vertInputInfo{.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO};

    // input assembly, we will be drawing triangle list
    VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
        .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST};

    // depth/stencil configuration
    VkPipelineDepthStencilStateCreateInfo depthStencilInfo{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
        .depthTestEnable = VK_TRUE,
        .depthWriteEnable = VK_TRUE,
        .depthCompareOp = VK_COMPARE_OP_LESS,
        .stencilTestEnable = VK_FALSE};

    // viewport
    VkPipelineViewportStateCreateInfo viewportInfo{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
        .viewportCount = 1,
        .pViewports = nullptr,
        .scissorCount = 1,
        .pScissors = nullptr};

    // rasterizer settings
    VkPipelineRasterizationStateCreateInfo rasterInfo{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
        .polygonMode = VK_POLYGON_MODE_FILL,
        .cullMode = VK_CULL_MODE_BACK_BIT,
        .frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE,
        .lineWidth = 1.0f};

    // no multisampling
    VkPipelineMultisampleStateCreateInfo multiSampleInfo{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
        .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT};

    // Alpha blending
    VkPipelineColorBlendAttachmentState attachState{
        .blendEnable = VK_FALSE,
        .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT};
    VkPipelineColorBlendStateCreateInfo blendInfo{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
        .attachmentCount = 1,
        .pAttachments = &attachState};

    // enable dynamic states
    std::vector<VkDynamicState> dynamicStates{VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
    VkPipelineDynamicStateCreateInfo dynamicStateInfo{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
        .dynamicStateCount = (uint32_t)dynamicStates.size(),
        .pDynamicStates = dynamicStates.data()};

    // structure required for dynamic rendering
    VkPipelineRenderingCreateInfo renderInfo{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO,
        .colorAttachmentCount = 1,
        .pColorAttachmentFormats = &cSwapchainFormat,
        .depthAttachmentFormat = cDepthFormat};

    // create the graphics pipeline
    VkGraphicsPipelineCreateInfo pipelineInfo{
        .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
        .pNext = &renderInfo,
        .stageCount = (uint32_t)shaderStages.size(),
        .pStages = shaderStages.data(),
        .pVertexInputState = &vertInputInfo,
        .pInputAssemblyState = &inputAssemblyInfo,
        .pViewportState = &viewportInfo,
        .pRasterizationState = &rasterInfo,
        .pMultisampleState = &multiSampleInfo,
        .pDepthStencilState = &depthStencilInfo,
        .pColorBlendState = &blendInfo,
        .pDynamicState = &dynamicStateInfo,
        .layout = mPipelineLayout,
        .renderPass = VK_NULL_HANDLE};
    if (vkCreateGraphicsPipelines(mDevice, nullptr, 1, &pipelineInfo, nullptr, &mPipeline) != VK_SUCCESS)
    {
        throw std::runtime_error("Error creating the pipeline");
    }

    std::cout << "Created graphics pipeline.\n";
}

void Context::createSyncResources()
{
    VkSemaphoreTypeCreateInfo semaphoreTypeInfo{
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO,
        .semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE,
        .initialValue = cMaxFramesInFlight};
    VkSemaphoreCreateInfo semaphoreInfo{
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
        .pNext = &semaphoreTypeInfo};
    if (vkCreateSemaphore(mDevice, &semaphoreInfo, nullptr, &timelineSemaphore) != VK_SUCCESS)
    {
        throw std::runtime_error("Unable to create the timeline semaphore");
    }

    // per-frame image-acquire semaphores
    for (FrameResources &res : mFrameResources)
    {
        // create the binary semaphores
        VkSemaphoreCreateInfo semaphoreInfo{.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};
        if (vkCreateSemaphore(mDevice, &semaphoreInfo, nullptr, &res.mImageAcquireSemaphore) != VK_SUCCESS)
        {
            throw std::runtime_error("Error creating the per-frame image acquire semaphore");
        }
    }

    std::cout << "Create per-image acquire semaphores with timeline type semaphore.\n";
}

void Context::createCommandBuffers()
{
    
}

void Context::destroySwapchain()
{
    for (VkImageView imgView : mSwapchainImageViews)
    {
        vkDestroyImageView(mDevice, imgView, nullptr);
    }
    mSwapchainImageViews.clear();
    // destroy render complete semaphores
    for (VkSemaphore &semaphore : mRenderCompleteSemaphores)
    {
        vkDestroySemaphore(mDevice, semaphore, nullptr);
    }
    mRenderCompleteSemaphores.clear();
    if (mSwapchain)
    {
        vkDestroySwapchainKHR(mDevice, mSwapchain, nullptr);
        mSwapchain = VK_NULL_HANDLE;
    }
    // destroy depth buffer
    if (mDepthImageView)
    {
        vkDestroyImageView(mDevice, mDepthImageView, nullptr);
        vmaDestroyImage(mAllocator, mDepthImage, mDepthImageAllocation);
        mDepthImageView = VK_NULL_HANDLE;
    }
}

void Context::createShaders()
{
    mVertShader = createShaderModule("shader.vert", shaderc_vertex_shader);
    mFragShader = createShaderModule("shader.frag", shaderc_fragment_shader);

    std::cout << "Created default shaders.\n";
}

void Context::shutdown()
{
    // clean up swapchain
    destroySwapchain();
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
    if (mDevice)
    {
        vkDestroyDevice(mDevice, nullptr);
        mDevice = VK_NULL_HANDLE;
    }
    if (mInstance)
    {
        vkDestroyInstance(mInstance, nullptr);
        mInstance = VK_NULL_HANDLE;
    }
    volkFinalize();
}