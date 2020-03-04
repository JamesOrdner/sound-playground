#include "VulkanFrame.h"
#include "VulkanDevice.h"
#include "VulkanMaterial.h"
#include "VulkanModel.h"
#include "VulkanMesh.h"
#include <stdexcept>

VulkanFrame::VulkanFrame(
	const VulkanDevice* device,
	VkCommandPool commandPool,
	VkDescriptorPool descriptorPool,
	VkDescriptorSetLayout descriptorSetLayout
) :
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
	
	initUniformBuffer(descriptorPool, descriptorSetLayout);
}

VulkanFrame::~VulkanFrame()
{
	vkDestroySemaphore(device->vkDevice(), completeSemaphore, nullptr);
	vkDestroyFence(device->vkDevice(), completeFence, nullptr);
	device->allocator().unmap(modelTransformUniformBuffer);
	device->allocator().destroyBuffer(modelTransformUniformBuffer);
}

void VulkanFrame::initUniformBuffer(VkDescriptorPool descriptorPool, VkDescriptorSetLayout descriptorSetLayout)
{
	// uniform buffer
	constexpr size_t maxModelCount = 128;
    
    VkPhysicalDeviceLimits limits = device->physicalDeviceProperties().limits;
    size_t minUboAlignment = limits.minUniformBufferOffsetAlignment;
	uniformBufferAlignment = sizeof(mat::mat4);
    if (minUboAlignment > 0) {
        uniformBufferAlignment = (uniformBufferAlignment + minUboAlignment - 1) & ~(minUboAlignment - 1);
    }
    
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
	
	// descriptor set
	VkDescriptorSetAllocateInfo descriptorSetAllocInfo{
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
		.descriptorPool = descriptorPool,
		.descriptorSetCount = 1,
		.pSetLayouts = &descriptorSetLayout
	};
	
	if (vkAllocateDescriptorSets(device->vkDevice(), &descriptorSetAllocInfo, &descriptorSet) != VK_SUCCESS) {
		throw std::runtime_error("failed to allocate descriptor sets!");
	}
	
	VkDescriptorBufferInfo descriptorBufferInfo{
		.buffer = modelTransformUniformBuffer.buffer,
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

void VulkanFrame::updateModelTransform(const VulkanModel& model, const mat::mat4& viewProjMatrix) const
{
	mat::mat4 transform = mat::t(viewProjMatrix * model.transform);
	uint32_t offset = model.modelID * static_cast<uint32_t>(uniformBufferAlignment);
	char* dest = static_cast<char*>(modelTransformUniformBufferData) + offset;
	std::copy_n(&transform, 1, reinterpret_cast<mat::mat4*>(dest));
}

void VulkanFrame::flushModelTransformUpdates() const
{
	device->allocator().flush(modelTransformUniformBuffer);
}

void VulkanFrame::beginRenderPass(VkRenderPass renderPass, VkFramebuffer framebuffer, const VkRect2D& renderArea)
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
}

void VulkanFrame::bindMaterial(const VulkanMaterial& material) const
{
	material.bind(commandBuffer);
}

void VulkanFrame::bindMesh(const VulkanMesh& mesh) const
{
	vkCmdBindVertexBuffers(commandBuffer, 0, 1, &mesh.vertexBuffer.buffer, &mesh.vertexDataOffset);
	vkCmdBindIndexBuffer(commandBuffer, mesh.vertexBuffer.buffer, mesh.indexBufferOffset, VK_INDEX_TYPE_UINT16);
}

void VulkanFrame::draw(const VulkanModel& model) const
{
	uint32_t dynamicOffset = model.modelID * static_cast<uint32_t>(uniformBufferAlignment);
	vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, model.getMaterial()->pipelineLayout, 0, 1, &descriptorSet, 1, &dynamicOffset);
	vkCmdDrawIndexed(commandBuffer, model.getMesh()->indexBuffer.size(), 1, 0, 0, 0);
}

void VulkanFrame::endRenderPass()
{
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
