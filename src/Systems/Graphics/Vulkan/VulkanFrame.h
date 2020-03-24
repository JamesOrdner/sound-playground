#pragma once

#include "VulkanAllocator.h"
#include "../../../Util/Matrix.h"
#include <vulkan/vulkan.h>
#include <string>
#include <vector>
#include <map>

class VulkanFrame
{
	struct SceneData {
		VulkanBuffer modelTransforms;
		void* modelTransformsData;
		
		VulkanBuffer modelShadowTransforms;
		void* modelShadowTransformsData;
		
		VulkanBuffer constants;
		void* constantsData;
		
		void init(const VulkanAllocator& allocator, VkDeviceSize transformsBufferSize);
		void deinit(const VulkanAllocator& allocator);
	};
	
	struct UIData {
		
	};
	
public:
	
	VulkanFrame(const class VulkanDevice* device, VkCommandPool commandPool);
	
	~VulkanFrame();
	
	void registerScene(const class VulkanScene* scene);
	void unregisterScene(const class VulkanScene* scene);
	
	/// Update all per-frame data for the scene
	void updateSceneData(const class VulkanScene* scene);
	
	/// Called when descriptor set layouts are changed, i.e. when a new materal is added
	void updateDescriptorSets(const std::vector<class VulkanMaterial*>& materials, const class VulkanShadow* shadow);
	
	/// Begin recording commands for this frame
	void beginFrame();
	
	/// Render a full scene, with (optional) UI. Pass nullptr to `ui` if rendering without UI.
	void render(
		const class VulkanScene* scene,
		const class VulkanUI* ui,
		class VulkanShadow* shadow,
		VkRenderPass sceneRenderPass,
		VkFramebuffer framebuffer,
		const VkRect2D& renderArea
	);
	
	/// End recording commands and submit to the graphics queue. Returns the
	/// semaphore which will be signaled when the command buffer finishes execution.
	VkSemaphore endFrame(VkSemaphore acquireSemaphore);
	
private:
	
	const class VulkanDevice* const device;
	
	VkCommandBuffer commandBuffer;
	
	/// Host-side command buffer completion synchronization
	VkFence completeFence;
	
	/// Device-side command buffer completion synchronization
	VkSemaphore completeSemaphore;
	
	VkDescriptorPool descriptorPool;
	
	uint32_t uboAlignment;
	
	/// Maps scenes to their per-frame data
	std::map<const class VulkanScene*, SceneData> sceneData;
	
	/// Maps material names to their corresponding descriptor set
	std::map<std::string, VkDescriptorSet> descriptorSets;
	
	void initDescriptorPool();
	void initUniformBuffer(VkDescriptorPool descriptorPool, VkDescriptorSetLayout descriptorSetLayout);
	
	void renderShadowPass(const class VulkanScene* scene, class VulkanShadow* shadow);
	void renderScene(const class VulkanScene* scene);
	void renderUI(const class VulkanUI* ui);
};
