#include "VulkanInstance.h"
#include <SDL_vulkan.h>
#include <stdexcept>

const std::vector<const char*> validationLayers{
#ifndef NDEBUG
	"VK_LAYER_KHRONOS_validation"
#endif
};

VulkanInstance::VulkanInstance(SDL_Window* window)
{
	initInstance();
	
	if (!SDL_Vulkan_CreateSurface(window, instance, &surface)) {
		throw std::runtime_error("Failed to create Vulkan surface!");
	}
}

VulkanInstance::~VulkanInstance()
{
	vkDestroySurfaceKHR(instance, surface, nullptr);
	vkDestroyInstance(instance, nullptr);
}

void VulkanInstance::initInstance()
{
	VkInstanceCreateInfo instanceInfo{
		.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
		.enabledLayerCount = static_cast<uint32_t>(validationLayers.size()),
		.ppEnabledLayerNames = validationLayers.data(),
//		.enabledExtensionCount = static_cast<uint32_t>(instanceExtensions.size()),
//		.ppEnabledExtensionNames = instanceExtensions.data()
	};
	
	if (vkCreateInstance(&instanceInfo, nullptr, &instance) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create VkInstance!");
	}
}
