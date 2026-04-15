#include "vwGUI.h"

void vw::GUI::createGUIContext(GLFWwindow *window, const utils::Context &context)
{
    // create descriptor pool
    VkDescriptorPoolSize poolSizes[] = {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 100};

    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    poolInfo.maxSets = 100;
    poolInfo.poolSizeCount = 1;
    poolInfo.pPoolSizes = poolSizes;

    VW_CHECK(vkCreateDescriptorPool(context.device, &poolInfo, nullptr, &descriptorPool));

    // initialize imgui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForVulkan(window, true);

    VkPipelineRenderingCreateInfo pipelineRenderingInfo{};
    pipelineRenderingInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
    pipelineRenderingInfo.colorAttachmentCount = 1;
    pipelineRenderingInfo.pColorAttachmentFormats = &context.swapchainFormat;

    ImGui_ImplVulkan_PipelineInfo pipelineInfo{};
    pipelineInfo.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
    pipelineInfo.PipelineRenderingCreateInfo = pipelineRenderingInfo;

    ImGui_ImplVulkan_InitInfo imguiVulkanInfo{};
    imguiVulkanInfo.Instance = context.instance;
    imguiVulkanInfo.PhysicalDevice = context.physicalDevice;
    imguiVulkanInfo.Device = context.device;
    imguiVulkanInfo.QueueFamily = context.graphicsQueueFamily;
    imguiVulkanInfo.Queue = context.graphicsQueue;
    imguiVulkanInfo.DescriptorPool = descriptorPool;
    imguiVulkanInfo.MinImageCount = 2;
    imguiVulkanInfo.ImageCount = static_cast<uint32_t>(context.swapchainImageCount);
    imguiVulkanInfo.PipelineInfoMain = pipelineInfo;
    imguiVulkanInfo.UseDynamicRendering = true;

    ImGui_ImplVulkan_Init(&imguiVulkanInfo);

    std::cout << "Created GUI context.\n";
}

void vw::GUI::destroyGUIContext(VkDevice device)
{
    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    vkDestroySampler(device, viewportSampler, nullptr);

    vkDestroyDescriptorPool(device, descriptorPool, nullptr);

    std::cout << "Destroyed GUI context.\n";
}
