#include "VulkanShadow.h"
#include "VulkanDevice.h"
#include "VulkanShader.h"
#include "VulkanMesh.h"
#include "../../../Util/Matrix.h"
#include <array>

constexpr uint32_t shadowMapDimensions = 2048;
constexpr VkFormat shadowMapFormat = VK_FORMAT_D16_UNORM;

VulkanShadow::VulkanShadow(const VulkanDevice* device) :
	device(device)
{
	renderArea = {.extent = { shadowMapDimensions, shadowMapDimensions }};
	
	VkImageCreateInfo imageInfo{
		.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
		.imageType = VK_IMAGE_TYPE_2D,
		.format = shadowMapFormat,
		.extent = { shadowMapDimensions, shadowMapDimensions, 1 },
		.mipLevels = 1,
		.arrayLayers = 1,
		.samples = VK_SAMPLE_COUNT_1_BIT,
		.tiling = VK_IMAGE_TILING_OPTIMAL,
		.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
		.sharingMode = VK_SHARING_MODE_EXCLUSIVE
	};
	
	image = device->allocator().createImage(imageInfo, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	
	VkImageViewCreateInfo imageViewInfo{
		.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
		.image = image.image,
		.viewType = VK_IMAGE_VIEW_TYPE_2D,
		.format = shadowMapFormat,
		.subresourceRange = {
			.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT,
			.baseMipLevel = 0,
			.levelCount = 1,
			.baseArrayLayer = 0,
			.layerCount = 1
		}
	};
	
	if (vkCreateImageView(device->vkDevice(), &imageViewInfo, nullptr, &imageView) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create VulkanShadow image view!");
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
		.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE
	};
	
	if (vkCreateSampler(device->vkDevice(), &samplerInfo, nullptr, &sampler) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create VulkanShadow sampler!");
	}
	
	// RenderPass
	
	VkAttachmentDescription attachment{
		.format = shadowMapFormat,
		.samples = VK_SAMPLE_COUNT_1_BIT,
		.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
		.storeOp = VK_ATTACHMENT_STORE_OP_STORE,
		.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
		.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
		.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
		.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL
	};
	
	VkAttachmentReference attachmentReference{
		.attachment = 0,
		.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
	};
	
	VkSubpassDescription subpass{
		.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
		.pDepthStencilAttachment = &attachmentReference
	};
	
	std::array<VkSubpassDependency, 2> dependencies{
		VkSubpassDependency{ // Optional?
			.srcSubpass = VK_SUBPASS_EXTERNAL,
			.dstSubpass = 0,
			.srcStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
			.dstStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
			.srcAccessMask = VK_ACCESS_SHADER_READ_BIT,
			.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
			.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT
		},
		VkSubpassDependency{
			.srcSubpass = 0,
			.dstSubpass = VK_SUBPASS_EXTERNAL,
			.srcStageMask = VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
			.dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
			.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
			.dstAccessMask = VK_ACCESS_SHADER_READ_BIT,
			.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT
		}
	};
	
	VkRenderPassCreateInfo renderPassInfo{
		.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
		.attachmentCount = 1,
		.pAttachments = &attachment,
		.subpassCount = 1,
		.pSubpasses = &subpass,
		.dependencyCount = static_cast<uint32_t>(dependencies.size()),
		.pDependencies = dependencies.data()
	};
	
	if (vkCreateRenderPass(device->vkDevice(), &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create VulkanShadow render pass!");
	}
	
	// Framebuffer
	
	VkFramebufferCreateInfo framebufferInfo{
		.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
		.renderPass = renderPass,
		.attachmentCount = 1,
		.pAttachments = &imageView,
		.width = shadowMapDimensions,
		.height = shadowMapDimensions,
		.layers = 1
	};
	
	if (vkCreateFramebuffer(device->vkDevice(), &framebufferInfo, nullptr, &framebuffer) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create VulkanShadow render pass!");
	}
	
	// Pipeline
	
	VulkanShader shader(device->vkDevice(), "shadow");
	
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
		.width = static_cast<float>(shadowMapDimensions),
		.height = static_cast<float>(shadowMapDimensions),
		.minDepth = 0.0f,
		.maxDepth = 1.0f
	};
	
	VkRect2D scissor{
		.offset = { 0, 0 },
		.extent = { shadowMapDimensions, shadowMapDimensions }
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
	
	VkDescriptorSetLayoutBinding descriptorSetLayoutBinding{
		.binding = 0,
		.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,
		.descriptorCount = 1,
		.stageFlags = VK_SHADER_STAGE_VERTEX_BIT
	};
	
	VkDescriptorSetLayoutCreateInfo descriptorSetLayoutInfo{
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
		.bindingCount = 1,
		.pBindings = &descriptorSetLayoutBinding
	};
	
	if (vkCreateDescriptorSetLayout(device->vkDevice(), &descriptorSetLayoutInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create Vulkan descriptor set layout!");
	}
	
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
		.layout = pipelineLayout,
		.renderPass = renderPass
	};
	
	if (vkCreateGraphicsPipelines(device->vkDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipeline) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create Vulkan pipeline!");
	}
}

VulkanShadow::~VulkanShadow()
{
	vkDestroyPipeline(device->vkDevice(), pipeline, nullptr);
	vkDestroyPipelineLayout(device->vkDevice(), pipelineLayout, nullptr);
	vkDestroyDescriptorSetLayout(device->vkDevice(), descriptorSetLayout, nullptr);
	vkDestroyFramebuffer(device->vkDevice(), framebuffer, nullptr);
	vkDestroyRenderPass(device->vkDevice(), renderPass, nullptr);
	vkDestroySampler(device->vkDevice(), sampler, nullptr);
	vkDestroyImageView(device->vkDevice(), imageView, nullptr);
	device->allocator().destroyImage(image);
}
