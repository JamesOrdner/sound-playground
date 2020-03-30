#include "VulkanInstance.h"
#include "VulkanDevice.h"
#include "VulkanSwapchain.h"
#include "VulkanFrame.h"
#include "VulkanScene.h"
#include "VulkanUI.h"
#include "VulkanPipelineLayout.h"
#include "VulkanMaterial.h"
#include "VulkanMesh.h"
#include "VulkanShadow.h"
#include <SDL_vulkan.h>
#include <stdexcept>

#include "VulkanTexture.h"

const std::vector<const char*> validationLayers{
#ifndef NDEBUG
	"VK_LAYER_KHRONOS_validation"
#endif
};

VulkanInstance::VulkanInstance(SDL_Window* window) :
	frameIndex(0),
	activeFrame(nullptr),
	activeSwapchainImageIndex(0),
	activeFrameAcquireSemaphore(VK_NULL_HANDLE)
{
	initInstance(window);
	
	if (!SDL_Vulkan_CreateSurface(window, instance, &surface)) {
		throw std::runtime_error("Failed to create Vulkan surface!");
	}
	
	device = std::make_unique<VulkanDevice>(instance, surface, validationLayers);
	swapchain = std::make_unique<VulkanSwapchain>(device.get(), surface);
	initRenderPass();
	swapchain->initFramebuffers(renderPass);
	initCommandPool();
	
	pipelineLayouts = std::make_unique<VulkanPipelineLayouts>(device->vkDevice());
	shadow = std::make_unique<VulkanShadow>(device.get());
	
	for (auto& frame : frames) {
		frame = std::make_unique<VulkanFrame>(device.get(), *pipelineLayouts, *shadow, commandPool);
	}
	
	// TEMP
	VulkanTexture(device.get(), "res/textures/ui.bmp");
}

VulkanInstance::~VulkanInstance()
{
	vkDeviceWaitIdle(device->vkDevice());
	
	uis.clear();
	scenes.clear();
	materials.clear();
	meshes.clear();
	
	for (auto& frame : frames) frame.reset();
	
	shadow.reset();
	pipelineLayouts.reset();
	
	vkDestroyCommandPool(device->vkDevice(), commandPool, nullptr);
	swapchain.reset();
	vkDestroyRenderPass(device->vkDevice(), renderPass, nullptr);
	device.reset();
	vkDestroySurfaceKHR(instance, surface, nullptr);
	vkDestroyInstance(instance, nullptr);
}

void VulkanInstance::initInstance(SDL_Window* window)
{
	VkApplicationInfo applicationInfo{
		.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
		.pApplicationName = "Sound Playground",
		.applicationVersion = VK_MAKE_VERSION(1, 0, 0),
		.apiVersion = VK_API_VERSION_1_2
	};
	
	auto extensions = requiredInstanceExtensions(window);
	VkInstanceCreateInfo instanceInfo{
		.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
		.pApplicationInfo = &applicationInfo,
		.enabledLayerCount = static_cast<uint32_t>(validationLayers.size()),
		.ppEnabledLayerNames = validationLayers.data(),
		.enabledExtensionCount = static_cast<uint32_t>(extensions.size()),
		.ppEnabledExtensionNames = extensions.data()
	};
	
	if (vkCreateInstance(&instanceInfo, nullptr, &instance) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create Vulkan instance!");
	}
}

std::vector<const char*> VulkanInstance::requiredInstanceExtensions(SDL_Window* window)
{
	std::vector<const char*> extensions;

	uint32_t sdlExtensionCount;
	SDL_Vulkan_GetInstanceExtensions(window, &sdlExtensionCount, nullptr);
	extensions.resize(sdlExtensionCount);
	SDL_Vulkan_GetInstanceExtensions(window, &sdlExtensionCount, extensions.data());

	if (!validationLayers.empty()) {
		// check validation layer support
		uint32_t layerCount;
		vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
		std::vector<VkLayerProperties> supportedLayers(layerCount);
		vkEnumerateInstanceLayerProperties(&layerCount, supportedLayers.data());
		for (const auto& wantedLayer : validationLayers) {
			bool bSupported = false;
			for (const auto& supportedLayer : supportedLayers) {
				if (strcmp(wantedLayer, supportedLayer.layerName) == 0) {
					bSupported = true;
					break;
				}
			}
			if (!bSupported) throw std::runtime_error("Unsupported Vulkan validation layer(s)!");
		}

		// all layers supported
		extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
	}

	return extensions;
}

void VulkanInstance::initRenderPass()
{
	VkAttachmentDescription colorAttachmentDesc{
		.format = swapchain->presentFormat(),
		.samples = VK_SAMPLE_COUNT_1_BIT,
		.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
		.storeOp = VK_ATTACHMENT_STORE_OP_STORE,
		.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
		.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
		.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
		.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
	};
	
	VkAttachmentDescription depthAttachmentDesc{
		.format = swapchain->depthFormat(),
		.samples = VK_SAMPLE_COUNT_1_BIT,
		.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
		.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
		.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
		.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
		.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
		.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
	};
	
	VkAttachmentReference colorAttachmentRef{
		.attachment = 0,
		.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
	};
	
	VkAttachmentReference depthAttachmentRef{
		.attachment = 1,
		.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
	};
	
	VkSubpassDescription subpassDesc{
		.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
		.colorAttachmentCount = 1,
		.pColorAttachments = &colorAttachmentRef,
		.pDepthStencilAttachment = &depthAttachmentRef
	};
	
	VkAttachmentDescription attachments[] = { colorAttachmentDesc, depthAttachmentDesc };
	VkRenderPassCreateInfo renderPassInfo{
		.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
		.attachmentCount = 2,
		.pAttachments = attachments,
		.subpassCount = 1,
		.pSubpasses = &subpassDesc
	};
	
	if (vkCreateRenderPass(device->vkDevice(), &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create Vulkan render pass!");
	}
}

void VulkanInstance::initCommandPool()
{
	VkCommandPoolCreateInfo commandPoolInfo{
		.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
		.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
		.queueFamilyIndex = device->queues().graphics.familyIndex
	};
	
	if (vkCreateCommandPool(device->vkDevice(), &commandPoolInfo, nullptr, &commandPool) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create Vulkan command pool!");
	}
}

VulkanScene* VulkanInstance::createScene()
{
	auto* scene = scenes.emplace_back(std::make_unique<VulkanScene>(this)).get();
	for (auto& frame : frames) frame->registerScene(scene);
	return scene;
}

void VulkanInstance::destroyScene(VulkanScene* scene)
{
	for (auto it = scenes.cbegin(); it != scenes.cend(); it++) {
		if (it->get() == scene) {
			for (auto& frame : frames) frame->unregisterScene(scene);
			scenes.erase(it);
			break;
		}
	}
}

VulkanUI* VulkanInstance::createUI()
{
	auto* ui = uis.emplace_back(std::make_unique<VulkanUI>(device.get(), renderPass, swapchain->extent())).get();
	for (auto& frame : frames) frame->registerUI(ui);
	return ui;
}

void VulkanInstance::destroyUI(VulkanUI* ui)
{
	for (auto it = uis.cbegin(); it != uis.cend(); it++) {
		if (it->get() == ui) {
			for (auto& frame : frames) frame->unregisterUI(ui);
			uis.erase(it);
			break;
		}
	}
}

VulkanMesh* VulkanInstance::sharedMesh(const std::string& filepath)
{
	if (meshes.find(filepath) == meshes.end()) {
		meshes[filepath] = std::make_unique<VulkanMesh>(device.get(), filepath);
	}
	return meshes[filepath].get();
}

VulkanMaterial* VulkanInstance::sharedMaterial(const std::string& name)
{
	if (materials.find(name) == materials.end()) {
		materials[name] = std::make_unique<VulkanMaterial>(device.get(), name, pipelineLayouts->getObjectLayout(), swapchain->extent(), renderPass);
	}
	return materials[name].get();
}

void VulkanInstance::beginFrame()
{
	activeFrameAcquireSemaphore = swapchain->acquireNextImage(activeSwapchainImageIndex);
	
	activeFrame = frames[frameIndex++].get();
	frameIndex %= frames.size();
	
	activeFrame->beginFrame();
}

void VulkanInstance::draw(VulkanScene* scene, VulkanUI* ui)
{
	activeFrame->updateSceneData(scene);
	if (ui) activeFrame->updateUIData(ui); 
	
	VkRect2D renderArea{.extent = swapchain->extent() };
	activeFrame->render(scene, ui, shadow.get(), renderPass, swapchain->framebuffer(activeSwapchainImageIndex), renderArea);
}

void VulkanInstance::endFrameAndPresent()
{
	// end command buffer recording and present
	VkSemaphore waitSemaphore = activeFrame->endFrame(activeFrameAcquireSemaphore);
	swapchain->present(activeSwapchainImageIndex, waitSemaphore);
	activeFrame = nullptr;
}
