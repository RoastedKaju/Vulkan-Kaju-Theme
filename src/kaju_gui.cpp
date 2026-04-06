#include "kaju_gui.h"

#include "device.h"
#include "swapchain.h"
#include "kaju_window.h"
#include "instance.h"

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_vulkan.h>
#include <imgui_internal.h>

void KajuGui::createGuiContext(Device &device, Swapchain &swapchain, KajuWindow &window, Instance &instance)
{
    std::cout << "Creating GUI context.\n";

    VkDescriptorPoolSize pool_sizes[] = {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1};

    VkDescriptorPoolCreateInfo pool_info{};
    pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    pool_info.maxSets = 1;
    pool_info.poolSizeCount = 1;
    pool_info.pPoolSizes = pool_sizes;

    if (vkCreateDescriptorPool(device.getDevice(), &pool_info, nullptr, &descriptor_pool) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create ImGUI descriptor pool");
    }

    // Render pass
    VkAttachmentDescription color_attachment{};
    color_attachment.format = swapchain.getFormat();
    color_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
    color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR; // change to LOAD when you have a scene behind
    color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    color_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    color_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED; // change to PRESENT when you have a scene behind
    color_attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference color_ref{};
    color_ref.attachment = 0;
    color_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &color_ref;

    VkSubpassDependency dependency{};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    // ImGUI initialization
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();

    ImGuiIO &io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    // Optional: enable multi-viewports
    // io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

    ImGui_ImplGlfw_InitForVulkan(window.getWindow(), true);

    // Tell ImGui what format the swapchain image is - required for dynamic rendering
    VkPipelineRenderingCreateInfo pipeline_rendering_info{};
    pipeline_rendering_info.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
    pipeline_rendering_info.colorAttachmentCount = 1;
    pipeline_rendering_info.pColorAttachmentFormats = &swapchain.getFormat();

    ImGui_ImplVulkan_PipelineInfo pipeline_info{};
    pipeline_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
    pipeline_info.PipelineRenderingCreateInfo = pipeline_rendering_info;

    ImGui_ImplVulkan_InitInfo imgui_vulkan_init_info{};
    imgui_vulkan_init_info.Instance = instance.getInstance();
    imgui_vulkan_init_info.PhysicalDevice = device.getPhysicalDevice();
    imgui_vulkan_init_info.Device = device.getDevice();
    imgui_vulkan_init_info.QueueFamily = device.getGraphicsQueueFamily();
    imgui_vulkan_init_info.Queue = device.getGraphicsQueue();
    imgui_vulkan_init_info.DescriptorPool = descriptor_pool;
    imgui_vulkan_init_info.MinImageCount = 2;
    imgui_vulkan_init_info.ImageCount = static_cast<uint32_t>(swapchain.getImages().size());
    imgui_vulkan_init_info.PipelineInfoMain = pipeline_info;
    imgui_vulkan_init_info.UseDynamicRendering = true;

    ImGui_ImplVulkan_Init(&imgui_vulkan_init_info);
}

void KajuGui::beginFrame()
{
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void KajuGui::endFrame(VkCommandBuffer command_buffer, Swapchain &swapchain, uint32_t swapchain_image_index)
{
    ImGui::Render();

    // For multi-viewport
    // ImGui::UpdatePlatformWindows();
    // ImGui::RenderPlatformWindowsDefault();

    VkClearValue clear_value{};
    clear_value.color = {{0.1f, 0.2f, 0.5f, 1.0f}};

    VkRenderingAttachmentInfo color_attachment{};
    color_attachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
    color_attachment.imageView = swapchain.getImageViews()[swapchain_image_index];
    color_attachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    color_attachment.clearValue = clear_value;

    VkRenderingInfo rendering_info{};
    rendering_info.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
    rendering_info.renderArea = {{0, 0}, swapchain.getExtent()};
    rendering_info.layerCount = 1;
    rendering_info.colorAttachmentCount = 1;
    rendering_info.pColorAttachments = &color_attachment;

    vkCmdBeginRendering(command_buffer, &rendering_info);
    ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), command_buffer);
    vkCmdEndRendering(command_buffer);
}

void KajuGui::destroyGuiContext(Device &device)
{
    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    vkDestroyDescriptorPool(device.getDevice(), descriptor_pool, nullptr);
}

void KajuGui::showDemo()
{
    ImGui::Begin("Demo", static_cast<bool *>(0), ImGuiWindowFlags_MenuBar);
    if (ImGui::BeginMenuBar())
    {
        if (ImGui::BeginMenu("File"))
        {
            if (ImGui::MenuItem("Open..", "Ctrl+O"))
            { /* Do stuff */
            }
            if (ImGui::MenuItem("Save", "Ctrl+S"))
            { /* Do stuff */
            }
            ImGui::EndMenu();
        }
        ImGui::EndMenuBar();
    }

    // Generate samples and plot them
    float samples[100];
    for (int n = 0; n < 100; n++)
    {
        samples[n] = sinf(n * 0.2f + (float)ImGui::GetTime() * 1.5f);
    }
    ImGui::PlotLines("Samples", samples, 100);

    // Display contents in a scrolling region
    ImGui::TextColored(ImVec4(1, 1, 0, 1), "Important Stuff");
    ImGui::BeginChild("Scrolling");
    for (int n = 0; n < 50; n++)
    {
        ImGui::Text("%04d: Some text", n);
    }
    ImGui::EndChild();
    ImGui::End();
    // Another window
    ImGui::Begin("Another Window");
    ImGui::Text("Hello, world %d", 123);
    ImGui::End();
}

void KajuGui::buildDockingLayout()
{
    // Setup docking space
    ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_PassthruCentralNode;
    ImGuiID dockspace_id = ImGui::DockSpaceOverViewport(ImGui::GetMainViewport()->ID, nullptr, dockspace_flags);

    static bool first_time = true;

    if (first_time)
    {
        first_time = false;

        // Clear existing layout for this ID
        ImGui::DockBuilderRemoveNode(dockspace_id);
        ImGui::DockBuilderAddNode(dockspace_id, ImGuiDockNodeFlags_DockSpace);
        ImGui::DockBuilderSetNodeSize(dockspace_id, ImGui::GetMainViewport()->Size);

        // Split the dockspace
        ImGuiID dock_main_id = dockspace_id;
        ImGuiID dock_id_left = ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Left, 0.20f, nullptr, &dock_main_id);
        ImGuiID dock_id_right = ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Right, 0.50f, nullptr, &dock_main_id);
        ImGuiID dock_id_bottom = ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Down, 0.20f, nullptr, &dock_main_id);

        // Assign windows to specific dock nodes
        ImGui::DockBuilderDockWindow("Scene", dock_main_id);
        ImGui::DockBuilderDockWindow("Demo", dock_id_left);
        ImGui::DockBuilderDockWindow("Another Window", dock_id_right);

        ImGui::DockBuilderFinish(dockspace_id);
    }
}
