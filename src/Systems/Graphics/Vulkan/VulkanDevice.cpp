#include "VulkanDevice.h"
#include "VulkanAllocator.h"
#include <array>
#include <optional>

const std::array<const char*, 1> requiredDeviceExtensions{
	VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

struct VulkanQueueFamilyIndices
{
	std::optional<uint32_t> graphicsFamily;
	std::optional<uint32_t> presentFamily;

	bool isComplete() {
		return graphicsFamily.has_value() && presentFamily.has_value();
	}

	std::vector<uint32_t> uniqueIndices() {
		std::vector<uint32_t> indices;
		if (!isComplete()) return indices;

		uint32_t graphics = graphicsFamily.value();
		uint32_t present = presentFamily.value();

		indices.push_back(graphics);
		if (graphics != present) indices.push_back(present);

		return indices;
	}
};

VulkanQueueFamilyIndices queryDeviceQueueFamilyIndices(VkPhysicalDevice device, VkSurfaceKHR surface)
{
	VulkanQueueFamilyIndices indices;

	uint32_t queueFamilyCount;
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);
	std::vector<VkQueueFamilyProperties> queueFamilyProperties(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilyProperties.data());

	for (uint32_t i = 0; i < queueFamilyCount; i++) {
		if (queueFamilyProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
			indices.graphicsFamily = i;
		}

		VkBool32 bPresentSupport;
		vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &bPresentSupport);
		if (bPresentSupport) {
			indices.presentFamily = i;
		}
	}

	return indices;
}

VulkanDevice::VulkanDevice(VkInstance instance, VkSurfaceKHR surface, const std::vector<const char*>& validationLayers)
{
	physicalDevice = optimalPhysicalDevice(instance, surface);
	if (physicalDevice == VK_NULL_HANDLE) {
		throw std::runtime_error("Failed to find suitable Vulkan physical device!");
	}

	std::vector<VkDeviceQueueCreateInfo> queueInfos;
	auto queueFamilyIndices = queryDeviceQueueFamilyIndices(physicalDevice, surface);
	float queuePriority = 1.f;
	for (uint32_t queueFamilyIndex : queueFamilyIndices.uniqueIndices()) {
		VkDeviceQueueCreateInfo queueCreateInfo{
			.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
			.queueFamilyIndex = queueFamilyIndex,
			.queueCount = 1,
			.pQueuePriorities = &queuePriority
		};
		queueInfos.push_back(queueCreateInfo);
	}

	VkDeviceCreateInfo deviceInfo{
		.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
		.queueCreateInfoCount = static_cast<uint32_t>(queueInfos.size()),
		.pQueueCreateInfos = queueInfos.data(),
		.enabledLayerCount = static_cast<uint32_t>(validationLayers.size()),
		.ppEnabledLayerNames = validationLayers.data(),
		.enabledExtensionCount = static_cast<uint32_t>(requiredDeviceExtensions.size()),
		.ppEnabledExtensionNames = requiredDeviceExtensions.data()
	};

	if (vkCreateDevice(physicalDevice, &deviceInfo, nullptr, &device) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create Vulkan device!");
	}

	// VkQueue(s)
	vulkanQueues.graphics.familyIndex = queueFamilyIndices.graphicsFamily.value();
	vulkanQueues.present.familyIndex = queueFamilyIndices.presentFamily.value();
	vkGetDeviceQueue(device, vulkanQueues.graphics.familyIndex, 0, &vulkanQueues.graphics.queue);
	vkGetDeviceQueue(device, vulkanQueues.present.familyIndex, 0, &vulkanQueues.present.queue);
	
	vulkanAllocator = std::make_unique<VulkanAllocator>(device, physicalDevice);
	
	// transfer command pool
	
	VkCommandPoolCreateInfo commandPoolInfo{
		.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
		.queueFamilyIndex = vulkanQueues.graphics.familyIndex
	};

	if (vkCreateCommandPool(device, &commandPoolInfo, nullptr, &transferPool) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create Vulkan transfer command pool!");
	}
}

VulkanDevice::~VulkanDevice()
{
	vkDestroyCommandPool(device, transferPool, nullptr);
	vulkanAllocator.reset();
	vkDestroyDevice(device, nullptr);
}

VkPhysicalDevice VulkanDevice::optimalPhysicalDevice(VkInstance instance, VkSurfaceKHR surface)
{
	uint32_t deviceCount;
	vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
	if (!deviceCount) throw std::runtime_error("No Vulkan-enabled graphics devices found!");
	std::vector<VkPhysicalDevice> devices(deviceCount);
	vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

	VkPhysicalDevice selectedDevice = VK_NULL_HANDLE;
	for (const auto& device : devices) {
		// check extension support
		uint32_t deviceExtensionCount;
		vkEnumerateDeviceExtensionProperties(device, nullptr, &deviceExtensionCount, nullptr);
		std::vector<VkExtensionProperties> deviceExtensions(deviceExtensionCount);
		vkEnumerateDeviceExtensionProperties(device, nullptr, &deviceExtensionCount, deviceExtensions.data());
		size_t matchingExtensions = 0;
		for (const auto& requiredExtension : requiredDeviceExtensions) {
			for (const auto& deviceExtension : deviceExtensions) {
				if (strcmp(requiredExtension, deviceExtension.extensionName) == 0) {
					matchingExtensions++;
					break;
				}
			}
		}
		if (matchingExtensions != requiredDeviceExtensions.size()) continue;

		// check swapchain support
		uint32_t surfaceFormatCount;
		uint32_t surfacePresentModeCount;
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &surfaceFormatCount, nullptr);
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &surfacePresentModeCount, nullptr);
		if (!surfaceFormatCount || !surfacePresentModeCount) continue;

		// check queue family support
		if (!queryDeviceQueueFamilyIndices(device, surface).isComplete()) continue;

		// device supports all requirements
		if (selectedDevice == VK_NULL_HANDLE) {
			selectedDevice = device;
		}
		else {
			// prefer discrete GPU if current selected device is integrated
			VkPhysicalDeviceProperties deviceProperties;
			vkGetPhysicalDeviceProperties(device, &deviceProperties);
			if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
				selectedDevice = device;
			}
		}
	}

	return selectedDevice;
}

VkPhysicalDeviceProperties VulkanDevice::physicalDeviceProperties() const
{
	VkPhysicalDeviceProperties properties;
    vkGetPhysicalDeviceProperties(physicalDevice, &properties);
    return properties;
}

VkSurfaceCapabilitiesKHR VulkanDevice::surfaceCapabilities(VkSurfaceKHR surface) const
{
	VkSurfaceCapabilitiesKHR capabilities = {};
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &capabilities);
	return capabilities;
}

std::vector<VkSurfaceFormatKHR> VulkanDevice::surfaceFormats(VkSurfaceKHR surface) const
{
	uint32_t formatCount;
	vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, nullptr);
	std::vector<VkSurfaceFormatKHR> formats(formatCount);
	vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, formats.data());
	return formats;
}

std::vector<VkPresentModeKHR> VulkanDevice::surfacePresentModes(VkSurfaceKHR surface) const
{
	uint32_t modeCount;
	vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &modeCount, nullptr);
	std::vector<VkPresentModeKHR> modes(modeCount);
	vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &modeCount, modes.data());
	return modes;
}

VkFormat VulkanDevice::firstSupportedFormat(const std::vector<VkFormat>& formats, VkImageTiling tiling, VkFormatFeatureFlags features) const
{
	for (VkFormat format : formats) {
		VkFormatProperties formatProperties;
		vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &formatProperties);

		if (tiling == VK_IMAGE_TILING_LINEAR && (formatProperties.linearTilingFeatures & features) == features) {
			return format;
		}
		else if (tiling == VK_IMAGE_TILING_OPTIMAL && (formatProperties.optimalTilingFeatures & features) == features) {
			return format;
		}
	}

	return VK_FORMAT_UNDEFINED;
}

VulkanBuffer VulkanDevice::transferToDevice(void* data, VkDeviceSize size, VkBufferUsageFlags usage) const
{
	VkBufferCreateInfo transferBufferInfo{
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size = size,
        .usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT
    };
	
    VmaAllocationCreateInfo transferBufferAllocInfo{
        .usage = VMA_MEMORY_USAGE_CPU_TO_GPU,
        .requiredFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
    };
	
	VulkanBuffer transferBuffer = vulkanAllocator->createBuffer(transferBufferInfo, transferBufferAllocInfo);
	
	void* mapped;
	vulkanAllocator->map(transferBuffer, &mapped);
	std::memcpy(mapped, data, size);
	vulkanAllocator->unmap(transferBuffer);
	
	VkBufferCreateInfo deviceBufferInfo{
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size = size,
        .usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | usage
    };
	
    VmaAllocationCreateInfo deviceBufferAllocInfo{
        .usage = VMA_MEMORY_USAGE_GPU_ONLY
    };
	
	VulkanBuffer deviceBuffer = vulkanAllocator->createBuffer(deviceBufferInfo, deviceBufferAllocInfo);
	
	VkCommandBufferAllocateInfo allocInfo{
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
		.commandPool = transferPool,
		.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
		.commandBufferCount = 1
	};
	
	VkCommandBufferBeginInfo beginInfo{
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
		.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT
	};
	
	VkCommandBuffer cmd;
	vkAllocateCommandBuffers(device, &allocInfo, &cmd);
	vkBeginCommandBuffer(cmd, &beginInfo);
	
	VkBufferCopy copyRegion{.size = size };
	vkCmdCopyBuffer(cmd, transferBuffer.buffer, deviceBuffer.buffer, 1, &copyRegion);
	
	vkEndCommandBuffer(cmd);

	VkSubmitInfo submitInfo{
		.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
		.commandBufferCount = 1,
		.pCommandBuffers = &cmd
	};

	vkQueueSubmit(vulkanQueues.graphics.queue, 1, &submitInfo, VK_NULL_HANDLE);
	vkQueueWaitIdle(vulkanQueues.graphics.queue);

	vkFreeCommandBuffers(device, transferPool, 1, &cmd);
	
	vulkanAllocator->destroyBuffer(transferBuffer);
	
	return deviceBuffer;
}
