#include "VulkanFrame.h"
#include "VulkanDevice.h"
#include "VulkanModel.h"
#include "VulkanMesh.h"
#include <stdexcept>

VulkanFrame::VulkanFrame(const VulkanDevice* device, VkCommandPool commandPool) :
	device(device),
	renderPass(VK_NULL_HANDLE),
	renderArea({})
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
}

VulkanFrame::~VulkanFrame()
{
	vkDestroySemaphore(device->vkDevice(), completeSemaphore, nullptr);
	vkDestroyFence(device->vkDevice(), completeFence, nullptr);
}

void VulkanFrame::updateRenderDependencies(VkRenderPass renderPass, const VkRect2D& renderArea)
{
	this->renderPass = renderPass;
	this->renderArea = renderArea;
}

VkSemaphore VulkanFrame::render(VkFramebuffer framebuffer, VkSemaphore acquireSemaphore, const std::vector<std::unique_ptr<class VulkanModel>>& models)
{
	vkWaitForFences(device->vkDevice(), 1, &completeFence, VK_TRUE, UINT64_MAX);
	vkResetFences(device->vkDevice(), 1, &completeFence);
	
	recordCommandBuffer(framebuffer);
	
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

void VulkanFrame::recordCommandBuffer(VkFramebuffer framebuffer)
{
	VkCommandBufferBeginInfo commandBufferBeginInfo{
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
		.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT
	};
	
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
	
	vkBeginCommandBuffer(commandBuffer, &commandBufferBeginInfo);
	vkCmdBeginRenderPass(commandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
	
	
	vkCmdEndRenderPass(commandBuffer);
	vkEndCommandBuffer(commandBuffer);
}
