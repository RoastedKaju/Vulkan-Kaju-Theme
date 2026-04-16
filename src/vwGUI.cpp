#include "vwGUI.h"
#include "vwSwapchain.h"

void vw::GUI::createGUIContext(GLFWwindow *window, const utils::Context &context)
{
    // create descriptor pool
    VkDescriptorPoolSize poolSizes[] = {{VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 100}};

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

    ImGuiIO &io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

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

void vw::GUI::beginFrame()
{
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void vw::GUI::endFrame(VkCommandBuffer cmd, Swapchain &swapchain, uint32_t swapchainImageIndex)
{
    ImGui::Render();

    ImGuiIO &io = ImGui::GetIO();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();
    }

    VkClearValue clear_value{};
    clear_value.color = {{0.1f, 0.2f, 0.5f, 1.0f}};

    VkRenderingAttachmentInfo attachementInfo{};
    attachementInfo.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
    attachementInfo.imageView = swapchain.getViews()[swapchainImageIndex];
    attachementInfo.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    attachementInfo.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachementInfo.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attachementInfo.clearValue = clear_value;

    VkRenderingInfo renderingInfo{};
    renderingInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
    renderingInfo.renderArea = {{0, 0}, swapchain.getExtent()};
    renderingInfo.layerCount = 1;
    renderingInfo.colorAttachmentCount = 1;
    renderingInfo.pColorAttachments = &attachementInfo;

    vkCmdBeginRendering(cmd, &renderingInfo);
    ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), cmd);
    vkCmdEndRendering(cmd);
}
