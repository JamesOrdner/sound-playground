#include "VulkanMesh.h"
#include "VulkanDevice.h"

/// VertexAttribute enum values are equal to the attribute offset within the vertex buffer
enum class VertexAttribute : size_t
{
    Position = 0,
    Color = 3,
	
	/// Stride of all attributes within the vertex buffer
	IndexStride = 6
};

VulkanMesh::VulkanMesh(const VulkanDevice* device, const std::string& filepath) :
	filepath(filepath),
	device(device)
{
	vertexDataOffset = 0;
	vertexData = {
		-0.5f, 0.5f, 0.f,  1.f, 0.f, 0.f,
		 0.5f, 0.5f, 0.f,  0.f, 1.f, 0.f,
		 0.f, -0.5f, 0.f,  0.f, 0.f, 1.f
	};
	
	indexBufferOffset = vertexData.size() * sizeof(float);
	indexBuffer = { 0, 1, 2 };
	
	VkDeviceSize requiredMemory = indexBufferOffset + indexBuffer.size() * sizeof(indexBuffer[0]);
	VkBufferCreateInfo vertexBufferInfo{
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size = requiredMemory,
        .usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT
    };
    VmaAllocationCreateInfo bufferAllocInfo{
        .usage = VMA_MEMORY_USAGE_CPU_TO_GPU,
        .requiredFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
    };
	vertexBuffer = device->allocator().createBuffer(vertexBufferInfo, bufferAllocInfo);
	
	void* data;
	device->allocator().map(vertexBuffer, &data);
	{
		char* dataPtr = reinterpret_cast<char*>(data);
		float* vertexDataDest = reinterpret_cast<float*>(dataPtr + vertexDataOffset);
		uint16_t* indexBufferDest = reinterpret_cast<uint16_t*>(dataPtr + indexBufferOffset);
		std::copy(vertexData.cbegin(), vertexData.cend(), vertexDataDest);
		std::copy(indexBuffer.cbegin(), indexBuffer.cend(), indexBufferDest);
	}
	device->allocator().unmap(vertexBuffer);
}

VulkanMesh::~VulkanMesh()
{
	device->allocator().destroyBuffer(vertexBuffer);
}

VkVertexInputBindingDescription VulkanMesh::inputBindingDescription()
{
    return VkVertexInputBindingDescription{
        .binding = 0,
        .stride = static_cast<uint32_t>(static_cast<size_t>(VertexAttribute::IndexStride) * sizeof(float)),
        .inputRate = VK_VERTEX_INPUT_RATE_VERTEX
    };
}

std::vector<VkVertexInputAttributeDescription> VulkanMesh::inputAttributeDescriptions()
{
    VkVertexInputAttributeDescription position{
        .location = 0,
        .binding = 0,
        .format = VK_FORMAT_R32G32B32_SFLOAT,
        .offset = static_cast<uint32_t>(static_cast<size_t>(VertexAttribute::Position) * sizeof(float))
    };

    VkVertexInputAttributeDescription color{
        .location = 1,
        .binding = 0,
        .format = VK_FORMAT_R32G32B32_SFLOAT,
        .offset = static_cast<uint32_t>(static_cast<size_t>(VertexAttribute::Color) * sizeof(float))
    };

    return { position, color };
}
