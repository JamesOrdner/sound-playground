#pragma once

#include "VulkanAllocator.h"
#include <vector>
#include <memory>

class VulkanUI
{
public:
	
	VulkanUI(class VulkanInstance* instance, const class VulkanDevice* device, VkRenderPass renderPass, const VkExtent2D& swapchainExtent);
	
	~VulkanUI();
	
	class VulkanUIObject* createUIObject();
	
	void deleteUIObject(class VulkanUIObject* object);
	
	/// Sort VulkanUIObjects' draw order. Never performed automatically.
	void sortDrawOrder();
	
	VkPipeline pipeline;
	VkPipelineLayout pipelineLayout;
	
	/// Objects are sorted by draw order, from back to front
	std::vector<std::unique_ptr<class VulkanUIObject>> objects;
	
private:
	
	class VulkanInstance* const instance;
	
	const class VulkanDevice* const device;
	
	VkDescriptorSetLayout descriptorSetLayout;
	
	void initDescriptors();
	void initPipeline(VkRenderPass renderPass, const VkExtent2D& swapchainExtent);
};
