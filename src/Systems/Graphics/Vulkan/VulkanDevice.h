#pragma once

#include <vulkan/vulkan.h>
#include <vector>
#include <optional>

class VulkanDevice
{
	struct VulkanQueue {
		VkQueue queue;
		uint32_t familyIndex;
	};

	struct VulkanQueues {
		VulkanQueue graphics;
		VulkanQueue present;
	};

public:

	VulkanDevice(VkInstance instance, VkSurfaceKHR surface, const std::vector<const char*>& validationLayers);

	~VulkanDevice();

private:

	VkDevice device;
	VkPhysicalDevice physicalDevice;
	VulkanQueues queues;

	/// Returns the most suitable physical device, or VK_NULL_HANDLE if none found
	VkPhysicalDevice optimalPhysicalDevice(VkInstance instance, VkSurfaceKHR surface);
};
