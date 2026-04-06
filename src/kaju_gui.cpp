#include "kaju_gui.h"
#include "device.h"
#include "swapchain.h"
#include "kaju_window.h"
#include "instance.h"

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_vulkan.h>

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

    VkRenderPassCreateInfo create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    create_info.attachmentCount = 1;
    create_info.pAttachments = &color_attachment;
    create_info.subpassCount = 1;
    create_info.pSubpasses = &subpass;
    create_info.dependencyCount = 1;
    create_info.pDependencies = &dependency;

    if (vkCreateRenderPass(device.getDevice(), &create_info, nullptr, &render_pass) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create ImGUI render pass.");
    }

    // Framebuffers
    frame_buffers.resize(swapchain.getImageViews().size());
    for (size_t i = 0; i < frame_buffers.size(); ++i)
    {
        VkImageView attachment = swapchain.getImageViews()[i];

        VkFramebufferCreateInfo frame_buffer_create_info{};
        frame_buffer_create_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        frame_buffer_create_info.renderPass = render_pass;
        frame_buffer_create_info.attachmentCount = 1;
        frame_buffer_create_info.pAttachments = &attachment;
        frame_buffer_create_info.width = swapchain.getExtent().width;
        frame_buffer_create_info.height = swapchain.getExtent().height;
        frame_buffer_create_info.layers = 1;

        if (vkCreateFramebuffer(device.getDevice(), &frame_buffer_create_info, nullptr, &frame_buffers[i]) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to create ImGUI frame buffer");
        }
    }

    // ImGUI initialization
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForVulkan(window.getWindow(), true);

    ImGui_ImplVulkan_PipelineInfo pipeline_info{};
    pipeline_info.RenderPass = render_pass;
    pipeline_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;

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

    ImGui_ImplVulkan_Init(&imgui_vulkan_init_info);
}

void KajuGui::beginFrame()
{
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void KajuGui::endFrame(VkCommandBuffer command_buffer, uint32_t swapchain_image_index)
{
    ImGui::Render();

    VkClearValue clear_value{};
    clear_value.color = {{0.1f, 0.2f, 0.5f, 1.0f}};

    VkRenderPassBeginInfo begin_info{};
    begin_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    begin_info.renderPass = render_pass;
    begin_info.framebuffer = frame_buffers[swapchain_image_index];
    begin_info.renderArea.offset = {0, 0};
    begin_info.renderArea.extent = VkExtent2D{800, 600}; // Fix this
    begin_info.clearValueCount = 1;
    begin_info.pClearValues = &clear_value;

    vkCmdBeginRenderPass(command_buffer, &begin_info, VK_SUBPASS_CONTENTS_INLINE);
    ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), command_buffer);
    vkCmdEndRenderPass(command_buffer);
}

void KajuGui::destroyGuiContext(Device &device)
{
    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    for (VkFramebuffer frame_buffer : frame_buffers)
    {
        vkDestroyFramebuffer(device.getDevice(), frame_buffer, nullptr);
    }

    vkDestroyRenderPass(device.getDevice(), render_pass, nullptr);
    vkDestroyDescriptorPool(device.getDevice(), descriptor_pool, nullptr);
}

void KajuGui::showDemo()
{
    // Create a window called "My First Tool", with a menu bar.
    ImGui::Begin("My First Tool", static_cast<bool *>(0), ImGuiWindowFlags_MenuBar);
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
}
