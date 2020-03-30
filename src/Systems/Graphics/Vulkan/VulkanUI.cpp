#include "VulkanUI.h"
#include "VulkanDevice.h"
#include "VulkanShader.h"
#include <stdexcept>
#include <array>

VulkanUI::VulkanUI(const class VulkanDevice* device, VkRenderPass renderPass, const VkExtent2D& swapchainExtent) :
	device(device)
{
	initDescriptors();
	initPipeline(renderPass, swapchainExtent);
}

VulkanUI::~VulkanUI()
{
	vkDestroyPipeline(device->vkDevice(), pipeline, nullptr);
	vkDestroyPipelineLayout(device->vkDevice(), pipelineLayout, nullptr);
	vkDestroyDescriptorSetLayout(device->vkDevice(), descriptorSetLayout, nullptr);
}

void VulkanUI::initDescriptors()
{
	VkDescriptorSetLayoutBinding descriptorSetLayoutBinding{
		0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT
	};
	
	VkDescriptorSetLayoutCreateInfo descriptorSetLayoutInfo{
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
		.bindingCount = 1,
		.pBindings = &descriptorSetLayoutBinding
	};
	
	if (vkCreateDescriptorSetLayout(device->vkDevice(), &descriptorSetLayoutInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create Vulkan descriptor set layout!");
	}
}

void VulkanUI::initPipeline(VkRenderPass renderPass, const VkExtent2D& swapchainExtent)
{
	VulkanShader shader(device->vkDevice(), "ui");
	
	VkVertexInputBindingDescription vertexInputBinding{
        .binding = 0,
        .stride = static_cast<uint32_t>(2 * sizeof(mat::vec2)),
        .inputRate = VK_VERTEX_INPUT_RATE_VERTEX
    };
	
	std::array<VkVertexInputAttributeDescription, 2> vertexInputAttributes{
		VkVertexInputAttributeDescription{ // position
			.location = 0,
			.binding = 0,
			.format = VK_FORMAT_R32G32_SFLOAT,
			.offset = 0
		},
		VkVertexInputAttributeDescription{ // uv
			.location = 1,
			.binding = 0,
			.format = VK_FORMAT_R32G32_SFLOAT,
			.offset = static_cast<uint32_t>(sizeof(mat::vec2))
		}
    };
	
	VkPipelineVertexInputStateCreateInfo vertexInputInfo{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
		.vertexBindingDescriptionCount = 1,
		.pVertexBindingDescriptions = &vertexInputBinding,
		.vertexAttributeDescriptionCount = static_cast<uint32_t>(vertexInputAttributes.size()),
		.pVertexAttributeDescriptions = vertexInputAttributes.data()
	};
	
	VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
		.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST
	};
	
	VkViewport viewport{
		.x = 0.0f,
		.y = 0.0f,
		.width = static_cast<float>(swapchainExtent.width),
		.height = static_cast<float>(swapchainExtent.height),
		.minDepth = 0.0f,
		.maxDepth = 1.0f
	};
	
	VkRect2D scissor{
		.offset = { 0, 0 },
		.extent = swapchainExtent
	};
	
	VkPipelineViewportStateCreateInfo viewportInfo{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
		.viewportCount = 1,
		.pViewports = &viewport,
		.scissorCount = 1,
		.pScissors = &scissor
	};
	
	VkPipelineRasterizationStateCreateInfo rasterizationInfo{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
		.polygonMode = VK_POLYGON_MODE_FILL,
		.cullMode = VK_CULL_MODE_BACK_BIT,
		.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE,
		.lineWidth = 1.0f
	};
	
	VkPipelineMultisampleStateCreateInfo multisampleInfo{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
		.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
	};
	
	VkPipelineDepthStencilStateCreateInfo depthStencilInfo{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
		.depthTestEnable = VK_FALSE,
		.depthWriteEnable = VK_FALSE
	};
	
	VkPipelineColorBlendAttachmentState colorBlendAttachment{
		.blendEnable = VK_TRUE,
		.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA,
		.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
		.colorBlendOp = VK_BLEND_OP_ADD,
		.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
		.dstAlphaBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA,
		.alphaBlendOp = VK_BLEND_OP_ADD,
		.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT
	};
	
	VkPipelineColorBlendStateCreateInfo colorBlendInfo{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
		.attachmentCount = 1,
		.pAttachments = &colorBlendAttachment
	};
	
	VkPipelineLayoutCreateInfo pipelineLayoutInfo{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
		.setLayoutCount = 1,
		.pSetLayouts = &descriptorSetLayout
	};
	
	if (vkCreatePipelineLayout(device->vkDevice(), &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create Vulkan pipeline layout!");
	}
	
	VkGraphicsPipelineCreateInfo pipelineInfo{
		.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
		.stageCount = static_cast<uint32_t>(shader.stages.size()),
		.pStages = shader.stages.data(),
		.pVertexInputState = &vertexInputInfo,
		.pInputAssemblyState = &inputAssemblyInfo,
		.pViewportState = &viewportInfo,
		.pRasterizationState = &rasterizationInfo,
		.pMultisampleState = &multisampleInfo,
		.pDepthStencilState = &depthStencilInfo,
		.pColorBlendState = &colorBlendInfo,
		.layout = pipelineLayout,
		.renderPass = renderPass,
		.subpass = 0
	};
	
	if (vkCreateGraphicsPipelines(device->vkDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipeline) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create Vulkan UI pipeline!");
	}
}
