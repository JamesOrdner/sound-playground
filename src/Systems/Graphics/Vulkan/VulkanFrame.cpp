#include "VulkanFrame.h"
#include "VulkanDevice.h"
#include "VulkanScene.h"
#include "VulkanMaterial.h"
#include "VulkanShadow.h"
#include "VulkanModel.h"
#include "VulkanMesh.h"
#include <stdexcept>

constexpr size_t maxModelCount = 128;

VulkanFrame::VulkanFrame(const VulkanDevice* device, VkCommandPool commandPool) :
	device(device)
{
	VkCommandBufferAllocateInfo commandBufferInfo{
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
		.commandPool = commandPool,
		.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
		.commandBufferCount = 1
	};
	
	if (vkAllocateCommandBuffers(device->vkDevice(), &commandBufferInfo, &commandBuffer) != VK_SUCCESS) {
		throw std::runtime_error("Failed to allocate Vulkan command buffers!");
	}
	
	VkFenceCreateInfo fenceInfo{
		.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
		.flags = VK_FENCE_CREATE_SIGNALED_BIT
	};
	
	if (vkCreateFence(device->vkDevice(), &fenceInfo, nullptr, &completeFence) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create Vulkan frame fence!");
	}
	
	VkSemaphoreCreateInfo semaphoreInfo{
		.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO
	};
	
	if (vkCreateSemaphore(device->vkDevice(), &semaphoreInfo, nullptr, &completeSemaphore) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create Vulkan frame semaphore!");
	}
	
	// DescriptorPool
	
	std::array<VkDescriptorPoolSize, 3> descriptorPoolSizes{
		VkDescriptorPoolSize{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 2 },
		VkDescriptorPoolSize{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1 },
		VkDescriptorPoolSize{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1 }
	};
	
	VkDescriptorPoolCreateInfo descriptorPoolInfo{
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
		.maxSets = 2, // one material + shadow pipeline
		.poolSizeCount = static_cast<uint32_t>(descriptorPoolSizes.size()),
		.pPoolSizes = descriptorPoolSizes.data()
	};
	
	if (vkCreateDescriptorPool(device->vkDevice(), &descriptorPoolInfo, nullptr, &descriptorPool) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create Vulkan descriptor pool!");
	}
	
	// UBO alignment
	
	VkPhysicalDeviceLimits limits = device->physicalDeviceProperties().limits;
    size_t minUboAlignment = limits.minUniformBufferOffsetAlignment;
	uniformBufferAlignment = sizeof(mat::mat4);
    if (minUboAlignment > 0) {
        uniformBufferAlignment = (uniformBufferAlignment + minUboAlignment - 1) & ~(minUboAlignment - 1);
    }
	
	// Uniform buffers
	
	VkDeviceSize bufferSize = maxModelCount * uniformBufferAlignment;
	
	VkBufferCreateInfo bufferInfo{
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size = bufferSize,
        .usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT
    };
	
    VmaAllocationCreateInfo bufferAllocInfo{
        .usage = VMA_MEMORY_USAGE_CPU_TO_GPU,
        .requiredFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_CACHED_BIT
    };
	
	modelTransformUniformBuffer = device->allocator().createBuffer(bufferInfo, bufferAllocInfo);
	device->allocator().map(modelTransformUniformBuffer, &modelTransformUniformBufferData);
	
	modelShadowUniformBuffer = device->allocator().createBuffer(bufferInfo, bufferAllocInfo);
	device->allocator().map(modelShadowUniformBuffer, &modelShadowUniformBufferData);
	
	// constants (projection) uniform buffer
	VkBufferCreateInfo constantsBufferInfo{
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size = static_cast<uint32_t>(sizeof(mat::mat4)),
        .usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT
    };
	
    VmaAllocationCreateInfo constantsBufferAllocInfo{
        .usage = VMA_MEMORY_USAGE_CPU_TO_GPU,
        .requiredFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
    };
	
	constantsUniformBuffer = device->allocator().createBuffer(constantsBufferInfo, constantsBufferAllocInfo);
	device->allocator().map(constantsUniformBuffer, &constantsUniformBufferData);
}

VulkanFrame::~VulkanFrame()
{
	vkDestroyDescriptorPool(device->vkDevice(), descriptorPool, nullptr);
	vkDestroySemaphore(device->vkDevice(), completeSemaphore, nullptr);
	vkDestroyFence(device->vkDevice(), completeFence, nullptr);
	device->allocator().unmap(constantsUniformBuffer);
	device->allocator().unmap(modelShadowUniformBuffer);
	device->allocator().unmap(modelTransformUniformBuffer);
	device->allocator().destroyBuffer(constantsUniformBuffer);
	device->allocator().destroyBuffer(modelShadowUniformBuffer);
	device->allocator().destroyBuffer(modelTransformUniformBuffer);
}

void VulkanFrame::updateDescriptorSets(const std::vector<VulkanMaterial*>& materials, const VulkanShadow* shadow)
{
	vkResetDescriptorPool(device->vkDevice(), descriptorPool, 0);
	descriptorSets.clear();
	
	for (const auto* material : materials) {
		VkDescriptorSetAllocateInfo descriptorSetAllocInfo{
			.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
			.descriptorPool = descriptorPool,
			.descriptorSetCount = 1,
			.pSetLayouts = &material->descriptorSetLayout
		};
		
		VkDescriptorSet descriptorSet;
		if (vkAllocateDescriptorSets(device->vkDevice(), &descriptorSetAllocInfo, &descriptorSet) != VK_SUCCESS) {
			throw std::runtime_error("failed to allocate descriptor sets!");
		}
		
		descriptorSets[material->name] = descriptorSet;
		
		VkDescriptorBufferInfo modelTransformDescriptorBufferInfo{
			.buffer = modelTransformUniformBuffer.buffer,
			.offset = 0,
			.range = uniformBufferAlignment
		};
		
		VkWriteDescriptorSet modelTransformDescriptorWrite{
			.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
			.dstSet = descriptorSet,
			.dstBinding = 0,
			.descriptorCount = 1,
			.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,
			.pBufferInfo = &modelTransformDescriptorBufferInfo
		};
		
		VkDescriptorBufferInfo constantsDescriptorBufferInfo{
			.buffer = constantsUniformBuffer.buffer,
			.offset = 0,
			.range = sizeof(mat::mat4)
		};
		
		VkWriteDescriptorSet constantsDescriptorWrite{
			.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
			.dstSet = descriptorSet,
			.dstBinding = 1,
			.descriptorCount = 1,
			.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
			.pBufferInfo = &constantsDescriptorBufferInfo
		};
		
		VkDescriptorImageInfo shadowSamplerDescriptorImageInfo{
			.sampler = shadow->sampler,
			.imageView = shadow->imageView,
			.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL
		};
		
		VkWriteDescriptorSet shadowSamplerDescriptorWrite{
			.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
			.dstSet = descriptorSet,
			.dstBinding = 2,
			.descriptorCount = 1,
			.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
			.pImageInfo = &shadowSamplerDescriptorImageInfo
		};
		
		std::array<VkWriteDescriptorSet, 3> writeDescriptorSets{
			modelTransformDescriptorWrite,
			constantsDescriptorWrite,
			shadowSamplerDescriptorWrite
		};
		
		vkUpdateDescriptorSets(device->vkDevice(), writeDescriptorSets.size(), writeDescriptorSets.data(), 0, nullptr);
	}
	
	// shadow
	
	VkDescriptorSetAllocateInfo descriptorSetAllocInfo{
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
		.descriptorPool = descriptorPool,
		.descriptorSetCount = 1,
		.pSetLayouts = &shadow->descriptorSetLayout
	};
	
	VkDescriptorSet descriptorSet;
	if (vkAllocateDescriptorSets(device->vkDevice(), &descriptorSetAllocInfo, &descriptorSet) != VK_SUCCESS) {
		throw std::runtime_error("failed to allocate descriptor sets!");
	}
	
	descriptorSets["shadow"] = descriptorSet;
	
	VkDescriptorBufferInfo descriptorBufferInfo{
		.buffer = modelShadowUniformBuffer.buffer,
		.offset = 0,
		.range = uniformBufferAlignment
	};
	
	VkWriteDescriptorSet descriptorWrite{
		.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
		.dstSet = descriptorSet,
		.dstBinding = 0,
		.dstArrayElement = 0,
		.descriptorCount = 1,
		.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,
		.pBufferInfo = &descriptorBufferInfo
	};
	
	vkUpdateDescriptorSets(device->vkDevice(), 1, &descriptorWrite, 0, nullptr);
}

void VulkanFrame::beginFrame()
{
	vkWaitForFences(device->vkDevice(), 1, &completeFence, VK_TRUE, UINT64_MAX);
	vkResetFences(device->vkDevice(), 1, &completeFence);
	
	VkCommandBufferBeginInfo commandBufferBeginInfo{
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
		.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT
	};
	
	vkBeginCommandBuffer(commandBuffer, &commandBufferBeginInfo);
}

void VulkanFrame::updateModelTransform(const VulkanModel& model, const mat::mat4& viewMatrix) const
{
	uint32_t offset = model.modelID * static_cast<uint32_t>(uniformBufferAlignment);
	
	mat::mat4 modelView = mat::t(viewMatrix * model.transform);
	char* dest = static_cast<char*>(modelTransformUniformBufferData) + offset;
	std::copy_n(&modelView, 1, reinterpret_cast<mat::mat4*>(dest));
	
	mat::mat4 lightView = mat::lookAt(mat::vec3{ 0.3f, 1.f, 0.1f }, mat::vec3());
	mat::mat4 lightViewProj = mat::ortho(-20, 20, -20, 20, -20, 20) * lightView;
	dest = static_cast<char*>(modelShadowUniformBufferData) + offset;
	std::copy_n(&lightViewProj, 1, reinterpret_cast<mat::mat4*>(dest));
}

void VulkanFrame::flushModelTransformUpdates() const
{
	device->allocator().flush(modelTransformUniformBuffer);
}

void VulkanFrame::updateProjectionMatrix(const mat::mat4& projectionMatrix) const
{
	mat::mat4 transposed = mat::t(projectionMatrix);
	std::copy_n(&transposed, 1, reinterpret_cast<mat::mat4*>(constantsUniformBufferData));
}

void VulkanFrame::renderShadowPass(const VulkanScene* scene, VulkanShadow* shadow)
{
	VkClearValue clearValue{
		.depthStencil = { 1.f, 0 }
	};
	
	VkRenderPassBeginInfo renderPassBeginInfo{
		.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
		.renderPass = shadow->renderPass,
		.framebuffer = shadow->framebuffer,
		.renderArea = shadow->renderArea,
		.clearValueCount = 1,
		.pClearValues = &clearValue
	};
	
	vkCmdBeginRenderPass(commandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, shadow->pipeline);
	
	VkDescriptorSet shadowDescriptorSet = descriptorSets["shadow"];
	for (const auto& model : scene->models) {
		const VulkanMesh* mesh = model->getMesh();
		uint32_t dynamicOffset = model->modelID * static_cast<uint32_t>(uniformBufferAlignment);
		vkCmdBindVertexBuffers(commandBuffer, 0, 1, &mesh->vertexBuffer.buffer, &mesh->vertexDataOffset);
		vkCmdBindIndexBuffer(commandBuffer, mesh->vertexBuffer.buffer, mesh->indexBufferOffset, VK_INDEX_TYPE_UINT16);
		vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, shadow->pipelineLayout, 0, 1, &shadowDescriptorSet, 1, &dynamicOffset);
		vkCmdDrawIndexed(commandBuffer, mesh->indexBuffer.size(), 1, 0, 0, 0);
	}
	
	vkCmdEndRenderPass(commandBuffer);
}

void VulkanFrame::renderMainPass(const VulkanScene* scene, VkRenderPass renderPass, VkFramebuffer framebuffer, const VkRect2D& renderArea)
{
	VkClearValue clearValues[] = {
		{.color = {.float32 = { 0.0f, 0.0f, 0.0f, 1.0f }}},
		{.depthStencil = { 1.f, 0 }}
	};
	
	VkRenderPassBeginInfo renderPassBeginInfo{
		.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
		.renderPass = renderPass,
		.framebuffer = framebuffer,
		.renderArea = renderArea,
		.clearValueCount = 2,
		.pClearValues = clearValues
	};
	
	vkCmdBeginRenderPass(commandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
	
	const VulkanMaterial* material = nullptr;
	const VulkanMesh* mesh = nullptr;
	VkDescriptorSet descriptorSet;
	for (const auto& model : scene->models) {
		if (material != model->getMaterial()) {
			material = model->getMaterial();
			if (material) {
				material->bind(commandBuffer);
				descriptorSet = descriptorSets.find(material->name)->second;
			}
		}
		
		if (mesh != model->getMesh()) {
			mesh = model->getMesh();
			if (mesh) {
				vkCmdBindVertexBuffers(commandBuffer, 0, 1, &mesh->vertexBuffer.buffer, &mesh->vertexDataOffset);
				vkCmdBindIndexBuffer(commandBuffer, mesh->vertexBuffer.buffer, mesh->indexBufferOffset, VK_INDEX_TYPE_UINT16);
			}
		}
		
		if (!material || !mesh) break;
		
		uint32_t dynamicOffset = model->modelID * static_cast<uint32_t>(uniformBufferAlignment);
		vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, material->pipelineLayout, 0, 1, &descriptorSet, 1, &dynamicOffset);
		vkCmdDrawIndexed(commandBuffer, mesh->indexBuffer.size(), 1, 0, 0, 0);
	}
	
	vkCmdEndRenderPass(commandBuffer);
}

VkSemaphore VulkanFrame::endFrame(VkSemaphore acquireSemaphore)
{
	vkEndCommandBuffer(commandBuffer);
	
	VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT };
	VkSubmitInfo submitInfo{
		.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
		.waitSemaphoreCount = 1,
		.pWaitSemaphores = &acquireSemaphore,
		.pWaitDstStageMask = waitStages,
		.commandBufferCount = 1,
		.pCommandBuffers = &commandBuffer,
		.signalSemaphoreCount = 1,
		.pSignalSemaphores = &completeSemaphore
	};
	
	vkQueueSubmit(device->queues().graphics.queue, 1, &submitInfo, completeFence);
	
	return completeSemaphore;
}
