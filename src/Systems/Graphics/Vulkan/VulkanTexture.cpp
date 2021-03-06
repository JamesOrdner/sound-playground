#include "VulkanTexture.h"
#include "VulkanDevice.h"
#include <SDL_surface.h>
#include <stdexcept>
#include <cassert>

VulkanTexture::VulkanTexture(const VulkanDevice* device, VkDescriptorSetLayout descriptorLayout, VkDescriptorPool descriptorPool, const std::string& filepath) :
	device(device)
{
	SDL_Surface* surface = SDL_LoadBMP(filepath.c_str());
	if (!surface) {
		throw std::runtime_error("Failed to load image: " + filepath);
	}
	
	assert(SDL_ISPIXELFORMAT_PACKED(surface->format->format));
	assert(SDL_PIXELLAYOUT(surface->format->format) == SDL_PACKEDLAYOUT_8888);
	
	VkFormat format;
	switch (SDL_PIXELORDER(surface->format->format)) {
	case SDL_PACKEDORDER_ARGB:
		format = VK_FORMAT_R8G8B8A8_UNORM;
		break;
	default:
		throw std::runtime_error("Unsupported image format: " + filepath);
	}
	
	VkDeviceSize size = surface->w * surface->h * surface->format->BytesPerPixel;
	
	VkImageCreateInfo imageInfo{
		.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
		.imageType = VK_IMAGE_TYPE_2D,
		.format = format,
		.extent = { static_cast<uint32_t>(surface->w), static_cast<uint32_t>(surface->h), 1 },
		.mipLevels = 1,
		.arrayLayers = 1,
		.samples = VK_SAMPLE_COUNT_1_BIT,
		.tiling = VK_IMAGE_TILING_OPTIMAL,
		.usage = VK_IMAGE_USAGE_SAMPLED_BIT,
		.sharingMode = VK_SHARING_MODE_EXCLUSIVE
	};
	
	VkImageViewCreateInfo imageViewInfo{
		.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
		.viewType = VK_IMAGE_VIEW_TYPE_2D,
		.format = format,
		.subresourceRange = {
			.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
			.baseMipLevel = 0,
			.levelCount = 1,
			.baseArrayLayer = 0,
			.layerCount = 1
		}
	};
	
	image = device->transferToDevice(surface->pixels, size, imageInfo, imageViewInfo.subresourceRange);
	
	SDL_FreeSurface(surface);
	
	imageViewInfo.image = image.image;
	
	if (vkCreateImageView(device->vkDevice(), &imageViewInfo, nullptr, &imageView) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create Vulkan ImageView for " + filepath);
	}
	
	VkSamplerCreateInfo samplerInfo{
		.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
		.magFilter = VK_FILTER_LINEAR,
		.minFilter = VK_FILTER_LINEAR,
		.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR,
		.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
		.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
		.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
		.mipLodBias = 0.f,
		.minLod = 0.f,
		.maxLod = 1.f,
		.borderColor = VK_BORDER_COLOR_INT_TRANSPARENT_BLACK
	};
	
	if (vkCreateSampler(device->vkDevice(), &samplerInfo, nullptr, &sampler) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create Vulkan sampler for " + filepath);
	}
	
	// descriptor set
	
	VkDescriptorSetAllocateInfo descriptorSetAllocInfo{
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
		.descriptorPool = descriptorPool,
		.descriptorSetCount = 1,
		.pSetLayouts = &descriptorLayout
	};
	
	if (vkAllocateDescriptorSets(device->vkDevice(), &descriptorSetAllocInfo, &descriptorSet) != VK_SUCCESS) {
		throw std::runtime_error("failed to allocate descriptor sets!");
	}
	
	VkDescriptorImageInfo descriptorImageInfo{
		.sampler = sampler,
		.imageView = imageView,
		.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
	};
	
	VkWriteDescriptorSet descriptorWrite{
		.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
		.dstSet = descriptorSet,
		.dstBinding = 0,
		.descriptorCount = 1,
		.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
		.pImageInfo = &descriptorImageInfo
	};
	
	vkUpdateDescriptorSets(device->vkDevice(), 1, &descriptorWrite, 0, nullptr);
}

VulkanTexture::~VulkanTexture()
{
	vkDestroySampler(device->vkDevice(), sampler, nullptr);
	vkDestroyImageView(device->vkDevice(), imageView, nullptr);
	device->allocator().destroyImage(image);
}
