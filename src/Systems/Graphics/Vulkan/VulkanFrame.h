#pragma once

#include "VulkanAllocator.h"
#include "../../../Util/Matrix.h"
#include <vulkan/vulkan.h>

class VulkanFrame
{
public:
	
	VulkanFrame(
		const class VulkanDevice* device,
		VkCommandPool commandPool,
		VkDescriptorPool descriptorPool,
		VkDescriptorSetLayout descriptorSetLayout
	);
	
	~VulkanFrame();
	
	/// Begin recording commands for this frame
	void beginFrame();
	
	/// Update the model's transformation matrix in the uniform buffer
	void updateModelTransform(const class VulkanModel& model, const mat::mat4& viewMatrix) const;
	
	/// Call after all model transforms have been updated
	void flushModelTransformUpdates() const;
	
	void updateProjectionMatrix(const mat::mat4& projectionMatrix) const;

	/// Begin a render pass
	void beginRenderPass(VkRenderPass renderPass, VkFramebuffer framebuffer, const VkRect2D& renderArea);
	
	/// Bind a pipeline to the current command buffer
	void bindMaterial(const class VulkanMaterial& material) const;
	
	/// Bind a vertex buffer to the current command buffer
	void bindMesh(const class VulkanMesh& mesh) const;
	
	/// Draw a model using the currently-bound pipeline
	void draw(const class VulkanModel& model) const;

	/// End the active render pass
	void endRenderPass();
	
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
	
	uint32_t uniformBufferAlignment;
	
	VulkanBuffer modelTransformUniformBuffer;
	void* modelTransformUniformBufferData;
	
	VulkanBuffer constantsUniformBuffer;
	void* constantsUniformBufferData;
	
	VkDescriptorSet descriptorSet;
	
	void initUniformBuffer(VkDescriptorPool descriptorPool, VkDescriptorSetLayout descriptorSetLayout);
};
