#include "PhysicsMesh.h"
#include <fx/gltf.h>

PhysicsMesh::PhysicsMesh(const std::string& filepath)
{
	fx::gltf::Document gltfDocument = filepath.ends_with(".glb") ? fx::gltf::LoadFromBinary(filepath) : fx::gltf::LoadFromText(filepath);
	const auto& scene = gltfDocument.scenes[gltfDocument.scene];
	for (const auto nodeIndex : scene.nodes) {
		const auto& node = gltfDocument.nodes[nodeIndex];
		if (node.name != "_ray") continue;
		const auto& mesh = gltfDocument.meshes[node.mesh];
        const auto& primitive = mesh.primitives[0]; // assume single primitive

        // vertex buffer
        for (const auto& attribute : primitive.attributes) {
			if (attribute.first != "POSITION") continue;
			
			// index buffer
			const auto& indicesAccessor = gltfDocument.accessors[primitive.indices];
			const auto& indicesBufferView = gltfDocument.bufferViews[indicesAccessor.bufferView];
			const auto& indicesBuffer = gltfDocument.buffers[indicesBufferView.buffer];
			uint32_t indicesBufferOffset = indicesAccessor.byteOffset + indicesBufferView.byteOffset;
			uint32_t indicesBufferStride = indicesBufferView.byteStride ? indicesBufferView.byteStride : sizeof(uint16_t);
			
			// vertex buffer
            const auto& verticesAccessor = gltfDocument.accessors[attribute.second];
			const auto& verticesBufferView = gltfDocument.bufferViews[verticesAccessor.bufferView];
			const auto& verticesBuffer = gltfDocument.buffers[verticesBufferView.buffer];

            // source (glTF file)
            uint32_t verticesBufferOffset = verticesAccessor.byteOffset + verticesBufferView.byteOffset;
            uint32_t verticesBufferStride = verticesBufferView.byteStride ? verticesBufferView.byteStride : sizeof(mat::vec3);
            
			for (uint32_t i = 0; i < indicesAccessor.count; i++) {
				uint32_t indicesByteOffset = indicesBufferOffset + indicesBufferStride * i;
				uint16_t vertexIndex = *reinterpret_cast<const uint16_t*>(indicesBuffer.data.data() + indicesByteOffset);
                uint32_t vertexByteOffset = verticesBufferOffset + verticesBufferStride * vertexIndex;
				this->mesh.push_back(*reinterpret_cast<const mat::vec3*>(verticesBuffer.data.data() + vertexByteOffset));
            }
        }
		break; // assume single node
	}
}

PhysicsMesh::~PhysicsMesh()
{
}

// Define static variable
std::map<std::string, std::unique_ptr<PhysicsMesh>> PhysicsMesh::meshes;

PhysicsMesh* PhysicsMesh::sharedMesh(const std::string& filepath)
{
	if (auto* existing = meshes[filepath].get()) {
		return existing;
	}
	else {
		auto& mesh = meshes[filepath] = std::make_unique<PhysicsMesh>(filepath);
		return mesh.get();
	}
}

const std::vector<mat::vec3>& PhysicsMesh::buffer() const
{
	return mesh;
}
