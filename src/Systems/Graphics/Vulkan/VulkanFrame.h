#pragma once

#include <vulkan/vulkan.h>

class VulkanFrame
{
public:
	
	VulkanFrame(const class VulkanDevice* device, VkCommandPool commandPool);
	
	~VulkanFrame();
	
	/// Begin recording commands for this frame
	void beginFrame(VkFramebuffer framebuffer, VkRenderPass renderPass, const VkRect2D& renderArea);
	
	/// Bind a pipeline to the current command buffer
	void bindMaterial(const class VulkanMaterial& material) const;
	
	/// Bind a vertex buffer to the current command buffer
	void bindMesh(const class VulkanMesh& mesh) const;
	
	/// Draw a list of models using the currently-bound pipeline
	void draw(const class VulkanModel& model) const;
	
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
