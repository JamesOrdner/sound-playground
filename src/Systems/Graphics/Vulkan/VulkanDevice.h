#pragma once

#include <vulkan/vulkan.h>
#include <vector>
#include <memory>

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
	
	inline VkDevice vkDevice() const { return device; }
	inline class VulkanAllocator* allocator() const { return vulkanAllocator.get(); }
	
	/// Returns the surface capabilities of the active device
	VkSurfaceCapabilitiesKHR surfaceCapabilities(VkSurfaceKHR surface) const;
	
	/// Returns all supported device surface formats of the active device
	std::vector<VkSurfaceFormatKHR> surfaceFormats(VkSurfaceKHR surface) const;
	
	/// Returns all supported device present modes of the active device
	std::vector<VkPresentModeKHR> surfacePresentModes(VkSurfaceKHR surface) const;
	
	/// Takes a vector of requested formats and returns the first supported format.
	/// Returns VK_FORMAT_UNDEFINED if none of the requested formats are supported.
	VkFormat firstSupportedFormat(const std::vector<VkFormat>& formats, VkImageTiling tiling, VkFormatFeatureFlags features) const;
	
private:

	VkDevice device;
	VkPhysicalDevice physicalDevice;
	VulkanQueues queues;
	
	std::unique_ptr<class VulkanAllocator> vulkanAllocator;

	/// Returns the most suitable physical device, or VK_NULL_HANDLE if none found
	VkPhysicalDevice optimalPhysicalDevice(VkInstance instance, VkSurfaceKHR surface);
};
