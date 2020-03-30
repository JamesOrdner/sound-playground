#pragma once

#include <vulkan/vulkan.h>
#include <vector>
#include <array>

enum class VulkanDescriptorSetType
{
	ModelTransform,
	ShadowSampler,
	ShadowTransform
};

struct VulkanPipelineLayout
{
	/// All descriptor set layouts used by the pipeline layout
	std::vector<VkDescriptorSetLayout> descriptorSetLayouts;
	VkPipelineLayout pipelineLayout;
};

/// Contains all descriptor set layouts and pipeline layouts used anywhere in the application.
/// Shaders must follow the descriptor sets and bindings as defined in the construtor.
class VulkanPipelineLayouts
{
public:
	
	VulkanPipelineLayouts(const VkDevice device);
	
	~VulkanPipelineLayouts();
	
	inline const VulkanPipelineLayout& getObjectLayout() const { return objectLayout; };
	
private:
	
	const VkDevice device;
	
	std::vector<VkDescriptorSetLayout> descriptorSetLayouts;
	
	VulkanPipelineLayout objectLayout;
};
