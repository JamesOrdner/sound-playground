#include "VulkanDevice.h"
#include <array>

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
	queues.graphics.familyIndex = queueFamilyIndices.graphicsFamily.value();
	queues.present.familyIndex = queueFamilyIndices.presentFamily.value();
	vkGetDeviceQueue(device, queues.graphics.familyIndex, 0, &queues.graphics.queue);
	vkGetDeviceQueue(device, queues.present.familyIndex, 0, &queues.present.queue);
}

VulkanDevice::~VulkanDevice()
{
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
