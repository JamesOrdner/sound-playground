#include "VulkanMesh.h"

/// VertexAttribute enum values are equal to the attribute offset within the vertex buffer
enum class VertexAttribute : size_t
{
    Position = 0,
    Color = 3,
	
	/// Stride of all attributes within the vertex buffer
	IndexStride = 6
};

VulkanMesh::VulkanMesh(const std::string& filepath) :
	filepath(filepath)
{
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
