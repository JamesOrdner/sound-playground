#pragma once

#include "VulkanAllocator.h"
#include <string>
#include <vector>

class VulkanMesh
{
public:
	
	VulkanMesh(const class VulkanDevice* device, const std::string& filepath);
	
	~VulkanMesh();
	
	VulkanMesh(const VulkanMesh&) = delete;
    VulkanMesh& operator=(const VulkanMesh&) = delete;
	
	const std::string filepath;
	
	std::vector<float> vertexData;
	std::vector<uint16_t> indexBuffer;
	VkDeviceSize vertexDataOffset;
	VkDeviceSize indexBufferOffset;
	
	VulkanBuffer vertexBuffer;
	
	static VkVertexInputBindingDescription inputBindingDescription();
	static std::vector<VkVertexInputAttributeDescription> inputAttributeDescriptions();
	
private:
	
	const class VulkanDevice* const device;
};
