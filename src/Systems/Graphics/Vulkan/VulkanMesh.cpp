#include "VulkanMesh.h"

enum class VertexAttributeType {
    Position,
    Color
};

inline size_t attributeIndexOffset(VertexAttributeType attribute)
{
    switch (attribute) {
    case VertexAttributeType::Position: return 0;
    case VertexAttributeType::Color:    return 3;
    default:
        return 0;
    }
}

inline size_t attributeIndexStride()
{
    return 6;
}

VulkanMesh::VulkanMesh(const std::string& filepath) :
	filepath(filepath)
{
}

VkVertexInputBindingDescription VulkanMesh::inputBindingDescription()
{
    return VkVertexInputBindingDescription{
        .binding = 0,
        .stride = static_cast<uint32_t>(attributeIndexStride() * sizeof(float)),
        .inputRate = VK_VERTEX_INPUT_RATE_VERTEX
    };
}

std::vector<VkVertexInputAttributeDescription> VulkanMesh::inputAttributeDescriptions()
{
    VkVertexInputAttributeDescription position{
        .location = 0,
        .binding = 0,
        .format = VK_FORMAT_R32G32B32_SFLOAT,
        .offset = static_cast<uint32_t>(attributeIndexOffset(VertexAttributeType::Position) * sizeof(float))
    };

    VkVertexInputAttributeDescription color{
        .location = 1,
        .binding = 0,
        .format = VK_FORMAT_R32G32B32_SFLOAT,
        .offset = static_cast<uint32_t>(attributeIndexOffset(VertexAttributeType::Color) * sizeof(float))
    };

    return { position, color };
}
