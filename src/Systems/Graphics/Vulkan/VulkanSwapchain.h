#pragma once

#include "VulkanAllocator.h"
#include <vulkan/vulkan.h>
#include <vector>

class VulkanSwapchain
{
public:
	
	VulkanSwapchain(const class VulkanDevice* device, VkSurfaceKHR surface);
	
	~VulkanSwapchain();
	
	const VkFormat& presentFormat() const { return imageFormat.format; }
	const VkFormat& depthFormat() const { return depthImageFormat; }
	const VkExtent2D& extent() const { return imageExtent; }
	
	/// This should be called immediately after creation of the RenderPass
	void initFramebuffers(VkRenderPass renderPass);
	
	/// Assigns the index of the next swapchain image, and returns
	/// a semaphore which signals the acquisition of the image
	VkSemaphore acquireNextImage(uint32_t& imageIndex);
	
	/// Submit an image for presentation
	void present(uint32_t imageIndex, VkSemaphore waitSemaphore);
	
	/// Get the framebuffer associated with the image index
	VkFramebuffer framebuffer(uint32_t imageIndex) const;
	
private:
	
	const class VulkanDevice* const device;
	
	VkSwapchainKHR swapchain;
	
	std::vector<VkImageView> imageViews;
	
	VulkanImage depthImage;
    VkImageView depthImageView;
    VkFormat depthImageFormat;
	
    std::vector<VkFramebuffer> framebuffers;
	
	std::vector<VkSemaphore> acquireSemaphores;
	size_t acquireIndex;
	
	VkExtent2D imageExtent;
	VkSurfaceFormatKHR imageFormat;
	
	void initSwapchain(VkSurfaceKHR surface);
	void initImageViews();
	void initDepthImage();
	void initSynchronization();
};
