#include "VulkanInstance.h"
#include "VulkanDevice.h"
#include "VulkanSwapchain.h"
#include "VulkanFrame.h"
#include "VulkanModel.h"
#include <SDL_vulkan.h>
#include <stdexcept>

const std::vector<const char*> validationLayers{
#ifndef NDEBUG
	"VK_LAYER_KHRONOS_validation"
#endif
};

VulkanInstance::VulkanInstance(SDL_Window* window) :
	frameIndex(0)
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
	
	for (auto& frame : frames) frame = std::make_unique<VulkanFrame>(device.get(), commandPool);
}

VulkanInstance::~VulkanInstance()
{
	vkDeviceWaitIdle(device->vkDevice());
	
	for (auto& frame : frames) frame.reset();
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

void VulkanInstance::renderFrame()
{
	uint32_t imageIndex;
	VkSemaphore acquireSemaphore = swapchain->acquireNextImage(imageIndex);
	
	auto& frame = frames[frameIndex++];
	frameIndex %= frames.size();
	
	VkRect2D renderArea{ .offset = {}, .extent = swapchain->extent() };
	frame->beginFrame(swapchain->framebuffer(imageIndex), renderPass, renderArea);
	// frame->bindPipeline();
	// frame->draw();
	VkSemaphore waitSemaphore = frame->endFrame(acquireSemaphore);
	
	swapchain->present(imageIndex, waitSemaphore);
}
