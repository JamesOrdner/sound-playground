#pragma once

#include <vma/vk_mem_alloc.h>
#include <string>
#include <vector>

class VulkanMesh
{
public:
	
	VulkanMesh(const std::string& filepath);
	
	VulkanMesh(const VulkanMesh&) = delete;
    VulkanMesh& operator=(const VulkanMesh&) = delete;
	
	const std::string filepath;
	
	static VkVertexInputBindingDescription inputBindingDescription();
	static std::vector<VkVertexInputAttributeDescription> inputAttributeDescriptions();
};
