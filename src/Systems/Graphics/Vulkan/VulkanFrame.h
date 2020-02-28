#pragma once

#include <vulkan/vulkan.h>
#include <vector>
#include <memory>

class VulkanFrame
{
public:
	
	VulkanFrame(const class VulkanDevice* device, VkCommandPool commandPool);
	
	~VulkanFrame();
	
	/// Begin recording commands for this frame
	void beginFrame(VkFramebuffer framebuffer, VkRenderPass renderPass, const VkRect2D& renderArea);
	
	/// Bind a pipeline to the actively-recording command buffer
	void bindPipeline(VkPipeline pipeline, VkPipelineBindPoint pipelineBindPoint);
	
	/// Draw a list of models using the currently-bound pipeline
	void draw(const std::vector<std::unique_ptr<class VulkanModel>>& models);
	
	/// End recording commands and submit to the graphics queue. Returns the
	/// semaphore which will be signaled when the command buffer finishes execution.
	VkSemaphore endFrame(VkSemaphore acquireSemaphore);
	
private:
	
	const class VulkanDevice* const device;
	
	VkCommandBuffer commandBuffer;
	
	/// Signaled when the command buffer is idle
	VkFence completeFence;
	
	/// Signaled when the command buffer has finished execution
	VkSemaphore completeSemaphore;
	
	void recordCommandBuffer(VkFramebuffer framebuffer);
};
