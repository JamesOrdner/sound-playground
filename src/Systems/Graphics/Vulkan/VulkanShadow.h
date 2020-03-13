#pragma once

#include "VulkanAllocator.h"

class VulkanShadow
{
public:
	
	VulkanShadow(const class VulkanDevice* device);
	
	~VulkanShadow();
	
	VkDescriptorSetLayout descriptorSetLayout;
	VkPipelineLayout pipelineLayout;
	
private:
	
	const class VulkanDevice* const device;
	
	VulkanImage image;
	VkImageView imageView;
	VkSampler sampler;
	VkRenderPass renderPass;
	VkFramebuffer framebuffer;
	VkPipeline pipeline;
};
