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
	
	initCommandPool();
	
	frames[0] = std::make_unique<VulkanFrame>(device.get(), commandPool);
	frames[1] = std::make_unique<VulkanFrame>(device.get(), commandPool);
}

VulkanInstance::~VulkanInstance()
{
	vkDeviceWaitIdle(device->vkDevice());
	
	for (auto& frame : frames) frame.reset();
	vkDestroyCommandPool(device->vkDevice(), commandPool, nullptr);
	swapchain.reset();
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
	
	VkSemaphore waitSemaphore = frames[frameIndex++]->draw(swapchain->framebuffer(imageIndex), acquireSemaphore, models);
	frameIndex %= frames.size();
	
	swapchain->present(imageIndex, waitSemaphore);
}
