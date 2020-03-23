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
};

class VulkanUI
{
public:
	
	VulkanUI(const class VulkanDevice* device, VkRenderPass renderPass);
	
	~VulkanUI();
	
	void update();
	
	void draw(VkCommandBuffer cmd);
	
	/// Objects are sorted by draw order, from back to front
	std::vector<std::unique_ptr<VulkanUIObject>> objects;
	
private:
	
	const class VulkanDevice* const device;
	
	VulkanBuffer vertexBuffer;
	
	VkDescriptorSetLayout descriptorSetLayout;
	
	VkPipelineLayout pipelineLayout;
	VkPipeline pipeline;
	
	void initDescriptors();
	void initPipeline(VkRenderPass renderPass);
};
