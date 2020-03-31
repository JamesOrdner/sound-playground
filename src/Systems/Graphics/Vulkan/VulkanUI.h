#pragma once

#include "VulkanAllocator.h"
#include "../../../Util/Matrix.h"
#include <vector>
#include <memory>

struct VulkanUIObject
{
	// all in NDC
	mat::vec2 position;
	mat::vec2 bounds;
	mat::vec2 uv_position;
	mat::vec2 uv_bounds;
	class VulkanTexture* texture;
	
	// draws from low to high
	uint32_t drawOrder;
	
	VulkanUIObject();
};

class VulkanUI
{
public:
	
	VulkanUI(const class VulkanDevice* device, VkRenderPass renderPass, const VkExtent2D& swapchainExtent);
	
	~VulkanUI();
	
	VulkanUIObject* createUIObject();
	
	void deleteUIObject(VulkanUIObject* object);
	
	/// Sort VulkanUIObjects' draw order. Never performed automatically.
	void sortDrawOrder();
	
	VkPipeline pipeline;
	VkPipelineLayout pipelineLayout;
	
	/// Objects are sorted by draw order, from back to front
	std::vector<std::unique_ptr<VulkanUIObject>> objects;
	
private:
	
	const class VulkanDevice* const device;
	
	VkDescriptorSetLayout descriptorSetLayout;
	
	void initDescriptors();
	void initPipeline(VkRenderPass renderPass, const VkExtent2D& swapchainExtent);
};
