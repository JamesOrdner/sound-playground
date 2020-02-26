#pragma once

#include "VulkanAllocator.h"
#include <vulkan/vulkan.h>
#include <vector>

class VulkanSwapchain
{
public:
	
	VulkanSwapchain(const class VulkanDevice* device, VkSurfaceKHR surface);
	
	~VulkanSwapchain();
	
private:
	
	const class VulkanDevice* const device;
	
	VkSwapchainKHR swapchain;
	
	std::vector<VkImageView> imageViews;
	
	VulkanImage depthImage;
    VkImageView depthImageView;
    VkFormat depthImageFormat;
	
	VkRenderPass renderPass;
	
    std::vector<VkFramebuffer> framebuffers;
	
	VkExtent2D extent;
	VkSurfaceFormatKHR imageFormat;
	
	void initSwapchain(VkSurfaceKHR surface);
	void initImageViews();
	void initDepthImage();
	void initRenderPass();
	void initFramebuffers();
};
