#pragma once

#include "VulkanAllocator.h"
#include <string>

class VulkanTexture
{
public:
		
	VulkanTexture(const class VulkanDevice* device, const std::string& filepath);
	
	~VulkanTexture();
	
	VkImageView imageView;
	VkSampler sampler;
	
private:
	
	const class VulkanDevice* const device;
	
	VulkanImage image;
};
