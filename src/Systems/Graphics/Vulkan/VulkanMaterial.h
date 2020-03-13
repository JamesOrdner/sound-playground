#pragma once

#include <vulkan/vulkan.h>
#include <string>

class VulkanMaterial
{
public:
	
	VulkanMaterial(const class VulkanDevice* device, const std::string& name, const VkExtent2D& swapchainExtent, VkRenderPass renderPass);
	
	~VulkanMaterial();
	
	const std::string name;
	
	VkDescriptorSetLayout descriptorSetLayout;
	VkPipelineLayout pipelineLayout;
	
	void bind(VkCommandBuffer cmd) const;
	
private:
	
	const class VulkanDevice* const device;
	
	VkPipeline pipeline;
};
