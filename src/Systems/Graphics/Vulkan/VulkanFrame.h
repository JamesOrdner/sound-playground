#pragma once

#include "VulkanAllocator.h"
#include "../../../Util/Matrix.h"
#include <vulkan/vulkan.h>
#include <vector>
#include <map>

class VulkanFrame
{
public:
	
	VulkanFrame(const class VulkanDevice* device, VkCommandPool commandPool);
	
	~VulkanFrame();
	
	/// Called when descriptor set layouts are changed, i.e. when a new materal is added
	void updateDescriptorSets(const std::vector<class VulkanMaterial*>& materials, const class VulkanShadow* shadow);
	
	/// Begin recording commands for this frame
	void beginFrame();
	
	/// Update the model's transformation matrix in the uniform buffer
	void updateModelTransform(const class VulkanModel& model, const mat::mat4& viewMatrix) const;
	
	/// Call after all model transforms have been updated
	void flushModelTransformUpdates() const;
	
	void updateProjectionMatrix(const mat::mat4& projectionMatrix) const;
	
	void renderShadowPass(const class VulkanScene* scene, class VulkanShadow* shadow);
	
	void renderMainPass(const class VulkanScene* scene, VkRenderPass renderPass, VkFramebuffer framebuffer, const VkRect2D& renderArea);
	
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
	
	VulkanBuffer modelShadowUniformBuffer;
	void* modelShadowUniformBufferData;
	
	VulkanBuffer constantsUniformBuffer;
	void* constantsUniformBufferData;
	
	VkDescriptorPool descriptorPool;
	
	/// Maps material names to their corresponding descriptor set
	std::map<std::string, VkDescriptorSet> descriptorSets;
	
	void initDescriptorPool();
	void initUniformBuffer(VkDescriptorPool descriptorPool, VkDescriptorSetLayout descriptorSetLayout);
};
