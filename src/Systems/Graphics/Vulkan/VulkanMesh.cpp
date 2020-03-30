#include "VulkanMesh.h"
#include "VulkanDevice.h"

/// VertexAttribute enum values are equal to the attribute offset within the vertex buffer
enum class VertexAttribute : size_t
{
    Position = 0,
    Normal = 3,
	
	/// Stride of all attributes within the vertex buffer
	IndexStride = 6
};

inline bool gltfAttributeValid(const std::string& attribute)
{
    return
        attribute == "POSITION" ||
        attribute == "NORMAL";
}

inline uint32_t attributeTypeScalarCount(fx::gltf::Accessor::Type type)
{
    using Type = fx::gltf::Accessor::Type;
    switch (type) {
    case Type::Scalar: return 1;
    case Type::Vec2:   return 2;
    case Type::Vec3:   return 3;
    case Type::Vec4:   return 4;
    default:           return 0;
    }
}

VulkanMesh::VulkanMesh(const VulkanDevice* device, const std::string& filepath) :
	filepath(filepath),
	device(device)
{
	fx::gltf::Document gltfDocument = filepath.ends_with(".glb") ? fx::gltf::LoadFromBinary(filepath) : fx::gltf::LoadFromText(filepath);
	
	gltfPreprocess(gltfDocument);
    gltfProcess(gltfDocument);
	
	indexBufferOffset = vertexData.size() * sizeof(float);
	
	VkDeviceSize requiredMemory = indexBufferOffset + indexBuffer.size() * sizeof(indexBuffer[0]);
	
	std::vector<uint8_t> data(requiredMemory);
	float* vertexDataDest = reinterpret_cast<float*>(data.data());
	uint16_t* indexBufferDest = reinterpret_cast<uint16_t*>(data.data() + indexBufferOffset);
	std::copy(vertexData.cbegin(), vertexData.cend(), vertexDataDest);
	std::copy(indexBuffer.cbegin(), indexBuffer.cend(), indexBufferDest);
	
	VkBufferUsageFlags usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
	vertexBuffer = device->transferToDevice(data.data(), requiredMemory, usage);
}

VulkanMesh::~VulkanMesh()
{
	device->allocator().destroyBuffer(vertexBuffer);
}

void VulkanMesh::gltfPreprocess(const fx::gltf::Document& gltfDocument)
{
    const auto& scene = gltfDocument.scenes[gltfDocument.scene];
	for (const auto nodeIndex : scene.nodes) {
		const auto& node = gltfDocument.nodes[nodeIndex];
		if (node.name == "_ray") continue;
		const auto& mesh = gltfDocument.meshes[node.mesh];
		const auto& primitive = mesh.primitives[0]; // assume single primitive
		assert(primitive.mode == fx::gltf::Primitive::Mode::Triangles);
		
		// index buffer
		assert(primitive.indices >= 0);
		const auto& indicesAccessor = gltfDocument.accessors[primitive.indices];
		assert(indicesAccessor.componentType == fx::gltf::Accessor::ComponentType::UnsignedShort);
		indexBuffer.resize(indicesAccessor.count);

		// vertex buffer
		for (const auto& attribute : primitive.attributes) {
			if (gltfAttributeValid(attribute.first)) {
				const auto& attributeAccessor = gltfDocument.accessors[attribute.second];
				assert(attributeAccessor.componentType == fx::gltf::Accessor::ComponentType::Float);
				assert(attributeAccessor.count <= UINT16_MAX);

				size_t size = attributeAccessor.count * static_cast<size_t>(VertexAttribute::IndexStride);
				if (vertexData.empty()) {
					vertexData.resize(size);
				}
				else {
					assert(vertexData.size() == size);
				}
			}
		}
		break; // assume single node
	}
}

void VulkanMesh::gltfProcess(const fx::gltf::Document& gltfDocument)
{
    const auto& scene = gltfDocument.scenes[gltfDocument.scene];
    for (const auto nodeIndex : scene.nodes) {
		const auto& node = gltfDocument.nodes[nodeIndex];
		if (node.name == "_ray") continue;
		const auto& mesh = gltfDocument.meshes[node.mesh];
        const auto& primitive = mesh.primitives[0]; // assume single primitive

        // index buffer
        if (!indexBuffer.empty()) {
            const auto& indicesAccessor = gltfDocument.accessors[primitive.indices];
            const auto& bufferView = gltfDocument.bufferViews[indicesAccessor.bufferView];
            const auto& buffer = gltfDocument.buffers[bufferView.buffer];
            uint32_t offset = indicesAccessor.byteOffset + bufferView.byteOffset;
            uint32_t stride = bufferView.byteStride ? bufferView.byteStride : sizeof(uint16_t);
            for (uint32_t i = 0; i < indicesAccessor.count; i++) {
                uint32_t bufferIndex = offset + stride * i;
                indexBuffer[i] = *reinterpret_cast<const uint16_t*>(buffer.data.data() + bufferIndex);
            }
        }

        // vertex buffer
        for (const auto& attribute : primitive.attributes) {
            VertexAttribute attributeType;
            if      (attribute.first == "POSITION") attributeType = VertexAttribute::Position;
            else if (attribute.first == "NORMAL")  attributeType = VertexAttribute::Normal;
            else continue;

            const auto& attributeAccessor = gltfDocument.accessors[attribute.second];
            const auto& bufferView = gltfDocument.bufferViews[attributeAccessor.bufferView];
            const auto& buffer = gltfDocument.buffers[bufferView.buffer];

            // source (glTF file)
            uint32_t scalarCount = attributeTypeScalarCount(attributeAccessor.type);
            uint32_t offset = attributeAccessor.byteOffset + bufferView.byteOffset;
            uint32_t stride = bufferView.byteStride ? bufferView.byteStride : scalarCount * sizeof(float);

            // dest (vertexData)
            size_t destOffset = static_cast<size_t>(attributeType);
			size_t destStride = static_cast<size_t>(VertexAttribute::IndexStride);
            
            for (uint32_t i = 0; i < attributeAccessor.count; i++) {
                uint32_t srcIndex = offset + stride * i;
                uint32_t destIndex = destOffset + destStride * i;
                auto* srcData = reinterpret_cast<const float*>(buffer.data.data() + srcIndex);
                auto* destData = vertexData.data() + destIndex;
                std::copy_n(srcData, scalarCount, destData);
            }
        }
		break; // assume single node
    }
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
	
    VkVertexInputAttributeDescription normal{
        .location = 1,
        .binding = 0,
        .format = VK_FORMAT_R32G32B32_SFLOAT,
        .offset = static_cast<uint32_t>(static_cast<size_t>(VertexAttribute::Normal) * sizeof(float))
    };
	
    return { position, normal };
}
