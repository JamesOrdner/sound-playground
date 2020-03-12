#include "VulkanMaterial.h"
#include "VulkanDevice.h"
#include "VulkanShader.h"
#include "VulkanMesh.h"
#include <stdexcept>

VulkanMaterial::VulkanMaterial(const VulkanDevice* device, const std::string& name, const VkExtent2D& swapchainExtent, VkRenderPass renderPass, VkDescriptorSetLayout descriptorSetLayout) :
	name(name),
	device(device)
{
	initPipeline(swapchainExtent, renderPass, descriptorSetLayout);
}

VulkanMaterial::~VulkanMaterial()
{
	vkDestroyPipeline(device->vkDevice(), pipeline, nullptr);
	vkDestroyPipelineLayout(device->vkDevice(), pipelineLayout, nullptr);
}

void VulkanMaterial::initPipeline(const VkExtent2D& swapchainExtent, VkRenderPass renderPass, VkDescriptorSetLayout descriptorSetLayout)
{
	VulkanShader shader(device->vkDevice(), name.c_str());
	
	auto inputBindingDescription = VulkanMesh::inputBindingDescription();
	auto inputAttributeDescriptions = VulkanMesh::inputAttributeDescriptions();
	
	VkPipelineVertexInputStateCreateInfo vertexInputInfo{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
		.vertexBindingDescriptionCount = 1,
		.pVertexBindingDescriptions = &inputBindingDescription,
		.vertexAttributeDescriptionCount = static_cast<uint32_t>(inputAttributeDescriptions.size()),
		.pVertexAttributeDescriptions = inputAttributeDescriptions.data()
	};
	
	VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
		.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
		.primitiveRestartEnable = VK_FALSE
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
		.depthTestEnable = VK_TRUE,
		.depthWriteEnable = VK_TRUE,
		.depthCompareOp = VK_COMPARE_OP_LESS,
		.depthBoundsTestEnable = VK_FALSE,
		.stencilTestEnable = VK_FALSE
	};
	
	VkPipelineColorBlendAttachmentState colorBlendAttachment{
		.blendEnable = VK_FALSE,
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
		throw std::runtime_error("Failed to create Vulkan pipeline!");
	}
}

void VulkanMaterial::bind(VkCommandBuffer cmd) const
{
	vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
}
