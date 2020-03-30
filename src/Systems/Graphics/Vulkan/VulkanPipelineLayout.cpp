#include "VulkanPipelineLayout.h"
#include "../../../Util/Matrix.h"

VulkanPipelineLayouts::VulkanPipelineLayouts(const VkDevice device) :
	device(device)
{
	// setup descriptor set layouts
	
	VkDescriptorSetLayout transformsDescriptorSetLayout;
	{
		std::array<VkDescriptorSetLayoutBinding, 2> bindings{
			VkDescriptorSetLayoutBinding{ 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1, VK_SHADER_STAGE_VERTEX_BIT },
			VkDescriptorSetLayoutBinding{ 1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT }
		};
		
		VkDescriptorSetLayoutCreateInfo descriptorLayoutInfo{
			.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
			.bindingCount = static_cast<uint32_t>(bindings.size()),
			.pBindings = bindings.data()
		};
		
		if (vkCreateDescriptorSetLayout(device, &descriptorLayoutInfo, nullptr, &transformsDescriptorSetLayout) != VK_SUCCESS) {
			throw std::runtime_error("Failed to create Vulkan descriptor set layout!");
		}
		
		descriptorSetLayouts.push_back(transformsDescriptorSetLayout);
	}
	
	VkDescriptorSetLayout combinedSamplerDescriptorSetLayout;
	{
		VkDescriptorSetLayoutBinding binding{
			0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT
		};

		VkDescriptorSetLayoutCreateInfo descriptorLayoutInfo{
			.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
			.bindingCount = 1,
			.pBindings = &binding
		};

		if (vkCreateDescriptorSetLayout(device, &descriptorLayoutInfo, nullptr, &combinedSamplerDescriptorSetLayout) != VK_SUCCESS) {
			throw std::runtime_error("Failed to create Vulkan descriptor set layout!");
		}
		
		descriptorSetLayouts.push_back(combinedSamplerDescriptorSetLayout);
	}
	
	// set up pipelines from descriptor set layouts
	
	// objectLayout
	{
		objectLayout.descriptorSetLayouts = {
			combinedSamplerDescriptorSetLayout,
			transformsDescriptorSetLayout
		};
		
		VkPushConstantRange pushConstantRange{
			.stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
			.offset = 0,
			.size = static_cast<uint32_t>(sizeof(mat::mat4))
		};
		
		VkPipelineLayoutCreateInfo pipelineLayoutInfo{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
			.setLayoutCount = static_cast<uint32_t>(objectLayout.descriptorSetLayouts.size()),
			.pSetLayouts = objectLayout.descriptorSetLayouts.data(),
			.pushConstantRangeCount = 1,
			.pPushConstantRanges = &pushConstantRange
		};
		
		if (vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &objectLayout.pipelineLayout) != VK_SUCCESS) {
			throw std::runtime_error("Failed to create Vulkan pipeline layout!");
		}
	}
}

VulkanPipelineLayouts::~VulkanPipelineLayouts()
{
	vkDestroyPipelineLayout(device, objectLayout.pipelineLayout, nullptr);
	
	for (auto& descriptorSetLayout : descriptorSetLayouts) {
		vkDestroyDescriptorSetLayout(device, descriptorSetLayout, nullptr);
	}
}
