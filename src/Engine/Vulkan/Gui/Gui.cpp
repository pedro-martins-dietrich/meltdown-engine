#include "Gui.hpp"

#include <imgui_impl_glfw.h>
#include <imgui_impl_vulkan.h>

#include "../../Utils/Logger.hpp"

mtd::Gui::Gui(const vk::Device& vulkanDevice, InputHandler& inputHandler)
	: guiDescriptorPool{vulkanDevice}, showGui{false}
{
	ImGui::CreateContext();
	ImGui::StyleColorsDark();

	ImGuiIO& io = ImGui::GetIO();
	io.IniFilename = NULL;

	createDescriptorPool();

	setInputCallbacks(inputHandler);
}

mtd::Gui::~Gui()
{
	ImGui_ImplVulkan_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
}

// Initializes ImGui
void mtd::Gui::init
(
	const Window& window,
	const vk::Instance& instance,
	const Device& device,
	const vk::RenderPass& renderPass,
	uint32_t framesInFlight
) const
{
	window.initImGuiForGLFW();

	ImGui_ImplVulkan_InitInfo imGuiInitInfo{};
	imGuiInitInfo.Instance = instance;
	imGuiInitInfo.PhysicalDevice = device.getPhysicalDevice();
	imGuiInitInfo.Device = device.getDevice();
	imGuiInitInfo.QueueFamily = device.getQueueFamilies().getGraphicsFamilyIndex();
	imGuiInitInfo.Queue = device.getGraphicsQueue();
	imGuiInitInfo.PipelineCache = nullptr;
	imGuiInitInfo.DescriptorPool = guiDescriptorPool.getDescriptorPool();
	imGuiInitInfo.Subpass = 0;
	imGuiInitInfo.MinImageCount = framesInFlight;
	imGuiInitInfo.ImageCount = framesInFlight;
	imGuiInitInfo.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
	imGuiInitInfo.UseDynamicRendering = false;
	imGuiInitInfo.ColorAttachmentFormat = {};
	imGuiInitInfo.Allocator = nullptr;
	imGuiInitInfo.CheckVkResultFn = checkVulkanResult;
	imGuiInitInfo.MinAllocationSize = 1048576U;

	ImGui_ImplVulkan_Init(&imGuiInitInfo, renderPass);
}

// Renders the GUI in the current frame
void mtd::Gui::renderGui(const vk::CommandBuffer& commandBuffer) const
{
	if(!showGui) return;

	ImGui_ImplVulkan_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	for(GuiWindow* pGuiWindow: guiWindows)
		pGuiWindow->renderGui();

	ImGui::Render();
	ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), commandBuffer, nullptr);
}

// Checks ImGui Vulkan results
void mtd::Gui::checkVulkanResult(VkResult result)
{
	if(result != VK_SUCCESS)
		LOG_ERROR("[ImGui] Vulkan result: %d", result);
}

// Creates the descriptor pool for ImGui
void mtd::Gui::createDescriptorPool()
{
	std::vector<PoolSizeData> poolSizesInfo;
	poolSizesInfo.resize(1);
	poolSizesInfo[0].descriptorCount = 1;
	poolSizesInfo[0].descriptorType = vk::DescriptorType::eCombinedImageSampler;

	guiDescriptorPool.createDescriptorPool
	(
		poolSizesInfo, vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet
	);
}

// Configures the input logic for the GUI
void mtd::Gui::setInputCallbacks(InputHandler& inputHandler)
{
	inputHandler.setInputCallback("default", "toggle_gui", [this](bool pressed)
	{
		static bool alreadyPressed = false;

		if(pressed && !alreadyPressed)
			showGui = !showGui;

		alreadyPressed = pressed;
	});
}
