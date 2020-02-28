#pragma once

#include <vulkan/vulkan.h>
#include <vector>
#include <memory>

class VulkanFrame
{
public:
	
	VulkanFrame(const class VulkanDevice* device, VkCommandPool commandPool);
	
	~VulkanFrame();
	
	/// This must be called before any calls to render(), and any time the dependencies are modified
	void updateRenderDependencies(VkRenderPass renderPass, const VkRect2D& renderArea);
	
	/// Submit a command buffer to the graphics queue. Returns the semaphore
	/// which will be signaled when the command buffer finishes execution.
	VkSemaphore render(VkFramebuffer framebuffer, VkSemaphore acquireSemaphore, const std::vector<std::unique_ptr<class VulkanModel>>& models);
	
private:
	
	const class VulkanDevice* const device;
	
	VkCommandBuffer commandBuffer;
	
	/// Signaled when the command buffer is idle
	VkFence completeFence;
	
	/// Signaled when the command buffer has finished execution
	VkSemaphore completeSemaphore;
	
	VkRenderPass renderPass;
	VkRect2D renderArea;
	
	void recordCommandBuffer(VkFramebuffer framebuffer);
};
