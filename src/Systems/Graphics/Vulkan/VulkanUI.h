#pragma once

#include <vulkan/vulkan.h>

class VulkanUI
{
public:
	
	VulkanUI(const class VulkanDevice* device, VkRenderPass renderPass);
	
	~VulkanUI();
	
	void draw(VkCommandBuffer cmd);
	
private:
	
	const class VulkanDevice* const device;
	
	VkDescriptorSetLayout descriptorSetLayout;
	
	VkPipelineLayout pipelineLayout;
	VkPipeline pipeline;
	
	void initDescriptors();
	void initPipeline(VkRenderPass renderPass);
};
