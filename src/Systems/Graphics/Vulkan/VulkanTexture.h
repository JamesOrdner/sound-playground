#pragma once

#include "VulkanAllocator.h"
#include <string>

class VulkanTexture
{
public:
		
	VulkanTexture(const class VulkanDevice* device, VkDescriptorSetLayout descriptorLayout, VkDescriptorPool descriptorPool, const std::string& filepath);
	
	~VulkanTexture();
	
	VkDescriptorSet descriptorSet;
	
private:
	
	const class VulkanDevice* const device;
	
	VulkanImage image;
	VkImageView imageView;
	VkSampler sampler;
};
