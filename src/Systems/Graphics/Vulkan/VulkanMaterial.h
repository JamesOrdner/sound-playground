#pragma once

#include <vulkan/vulkan.h>
#include <string>

class VulkanMaterial
{
public:
	
	VulkanMaterial(const class VulkanDevice* device, const std::string& name, const struct VulkanPipelineLayout& layout, const VkExtent2D& swapchainExtent, VkRenderPass renderPass);
	
	~VulkanMaterial();
	
	const std::string name;
	
	const struct VulkanPipelineLayout& layout;
	
	void bind(VkCommandBuffer cmd) const;
	
private:
	
	const class VulkanDevice* const device;
	
	VkPipeline pipeline;
};
