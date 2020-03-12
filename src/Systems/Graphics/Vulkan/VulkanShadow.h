#pragma once

#include "VulkanAllocator.h"

class VulkanShadow
{
public:
	
	VulkanShadow(const class VulkanDevice* device);
	
	~VulkanShadow();
	
private:
	
	const class VulkanDevice* const device;
	
	VulkanImage image;
	VkImageView imageView;
	VkSampler sampler;
	VkRenderPass renderPass;
	VkFramebuffer framebuffer;
};
