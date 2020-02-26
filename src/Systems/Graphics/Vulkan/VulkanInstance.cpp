#include "VulkanInstance.h"
#include "VulkanDevice.h"
#include <SDL_vulkan.h>
#include <stdexcept>

const std::vector<const char*> validationLayers{
#ifndef NDEBUG
	"VK_LAYER_KHRONOS_validation"
#endif
};

VulkanInstance::VulkanInstance(SDL_Window* window)
{
	VkApplicationInfo appInfo{
		.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
		.pApplicationName = "Sound Playground",
		.applicationVersion = VK_MAKE_VERSION(1, 0, 0),
		.apiVersion = VK_API_VERSION_1_2
	};

	auto extensions = requiredInstanceExtensions(window);
	VkInstanceCreateInfo instanceInfo{
		.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
		.pApplicationInfo = &appInfo,
		.enabledLayerCount = static_cast<uint32_t>(validationLayers.size()),
		.ppEnabledLayerNames = validationLayers.data(),
		.enabledExtensionCount = static_cast<uint32_t>(extensions.size()),
		.ppEnabledExtensionNames = extensions.data()
	};

	if (vkCreateInstance(&instanceInfo, nullptr, &instance) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create VkInstance!");
	}
	
	if (!SDL_Vulkan_CreateSurface(window, instance, &surface)) {
		throw std::runtime_error("Failed to create Vulkan surface!");
	}

	device = std::make_unique<VulkanDevice>(instance, surface, validationLayers);
}

VulkanInstance::~VulkanInstance()
{
	device.reset();
	vkDestroySurfaceKHR(instance, surface, nullptr);
	vkDestroyInstance(instance, nullptr);
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
			if (!bSupported) throw std::runtime_error("Unsupported validation layer(s)!");
		}

		// all layers supported
		extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
	}

	return extensions;
}
