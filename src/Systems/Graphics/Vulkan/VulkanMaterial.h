#pragma once

#include <vulkan/vulkan.h>

class VulkanMaterial
{
public:
	
	VulkanMaterial(const class VulkanDevice* device, const VkExtent2D& swapchainExtent, VkRenderPass renderPass);
	
	~VulkanMaterial();
	
	void bind(VkCommandBuffer cmd) const;
	
private:
	
	const class VulkanDevice* const device;
	
	VkDescriptorSetLayout descriptorSetLayout;
	VkPipelineLayout pipelineLayout;
	VkPipeline pipeline;
	
	void initPipeline(const VkExtent2D& swapchainExtent, VkRenderPass renderPass);
};
