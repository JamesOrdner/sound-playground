#pragma once

#include <vma/vk_mem_alloc.h>
#include <vector>

class VulkanMesh
{
public:
	
	VulkanMesh(const VulkanMesh&) = delete;
    VulkanMesh& operator=(const VulkanMesh&) = delete;
	
	static VkVertexInputBindingDescription inputBindingDescription();
	static std::vector<VkVertexInputAttributeDescription> inputAttributeDescriptions();
};
