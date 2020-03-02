#pragma once

#include <vulkan/vulkan.h>
#include <string>

class VulkanMaterial
{
public:
	
	VulkanMaterial(const class VulkanDevice* device, const std::string& name, const VkExtent2D& swapchainExtent, VkRenderPass renderPass);
	
	~VulkanMaterial();
	
	const std::string name;
	
	void bind(VkCommandBuffer cmd) const;
	
private:
	
	const class VulkanDevice* const device;
	
	VkDescriptorSetLayout descriptorSetLayout;
	VkPipelineLayout pipelineLayout;
	VkPipeline pipeline;
	
	void initPipeline(const VkExtent2D& swapchainExtent, VkRenderPass renderPass);
};
