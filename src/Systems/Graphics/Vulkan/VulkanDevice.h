#pragma once

#include "VulkanAllocator.h"
#include <vector>
#include <memory>

struct VulkanQueue {
	VkQueue queue;
	uint32_t familyIndex;
};

struct VulkanQueues {
	VulkanQueue graphics;
	VulkanQueue present;
};

class VulkanDevice
{
public:

	VulkanDevice(VkInstance instance, VkSurfaceKHR surface, const std::vector<const char*>& validationLayers);

	~VulkanDevice();
	
	inline VkDevice vkDevice() const { return device; }
	inline VulkanAllocator& allocator() const { return *vulkanAllocator; }
	inline const VulkanQueues& queues() const { return vulkanQueues; }
	
	VkPhysicalDeviceProperties physicalDeviceProperties() const;
	
	/// Returns the surface capabilities of the active device
	VkSurfaceCapabilitiesKHR surfaceCapabilities(VkSurfaceKHR surface) const;
	
	/// Returns all supported device surface formats of the active device
	std::vector<VkSurfaceFormatKHR> surfaceFormats(VkSurfaceKHR surface) const;
	
	/// Returns all supported device present modes of the active device
	std::vector<VkPresentModeKHR> surfacePresentModes(VkSurfaceKHR surface) const;
	
	/// Takes a vector of requested formats and returns the first supported format.
	/// Returns VK_FORMAT_UNDEFINED if none of the requested formats are supported.
	VkFormat firstSupportedFormat(const std::vector<VkFormat>& formats, VkImageTiling tiling, VkFormatFeatureFlags features) const;
	
	/// Transfer buffer data to device-local memory, returning the device buffer
	VulkanBuffer transferToDevice(void* data, VkDeviceSize size, VkBufferUsageFlags usage) const;
	
	/// Transfer image data to device-local memory, returning the device buffer
	VulkanImage transferToDevice(void* data, VkDeviceSize size, VkImageCreateInfo& imageInfo, const VkImageSubresourceRange& subresourceRange) const;
	
private:

	VkDevice device;
	VkPhysicalDevice physicalDevice;
	VulkanQueues vulkanQueues;
	
	std::unique_ptr<VulkanAllocator> vulkanAllocator;
	
	VkCommandPool transferPool;

	/// Returns the most suitable physical device, or VK_NULL_HANDLE if none found
	VkPhysicalDevice optimalPhysicalDevice(VkInstance instance, VkSurfaceKHR surface);
};
