#include "PhysicsMesh.h"

PhysicsMesh::PhysicsMesh(const std::string& filepath)
{
//	using namespace tinygltf;
//
//	TinyGLTF loader;
//	Model model;
//	std::string err;
//	std::string warn;
//	loader.LoadBinaryFromFile(&model, &err, &warn, filepath);
//
//	for (const Node& node : model.nodes) {
//		if (node.name != "_ray") continue;
//
//		// There should be only one primitive
//		const Primitive& primitive = model.meshes[node.mesh].primitives[0];
//
//		// Index array buffer
//		Accessor idx_accessor = model.accessors[primitive.indices];
//		const BufferView& idx_bufferView = model.bufferViews[idx_accessor.bufferView];
//		const Buffer& idx_buffer = model.buffers[idx_bufferView.buffer];
//
//		for (auto& attrib : primitive.attributes) {
//			if (attrib.first != "POSITION") continue;
//
//			// Vertex buffer
//			const Accessor& vert_accessor = model.accessors[attrib.second];
//			const BufferView& vert_bufferView = model.bufferViews[vert_accessor.bufferView];
//			const Buffer& vert_buffer = model.buffers[vert_bufferView.buffer];
//
//			assert(primitive.mode == TINYGLTF_MODE_TRIANGLES);
//			assert(idx_accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT);
//			assert(vert_accessor.type == TINYGLTF_TYPE_VEC3);
//			assert(vert_accessor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT);
//
//			mesh.reserve(idx_accessor.count);
//
//			size_t idx_stride = idx_accessor.ByteStride(idx_bufferView);
//			size_t vert_stride = vert_accessor.ByteStride(vert_bufferView);
//			for (size_t i = 0; i < idx_accessor.count; i++) {
//				const unsigned char* iptr = &idx_buffer.data[0] + idx_bufferView.byteOffset + idx_stride * i;
//				int v_idx = static_cast<int>(*((unsigned short*)iptr));
//				const unsigned char* vptr = &vert_buffer.data[0] + vert_bufferView.byteOffset + vert_stride * v_idx;
//				mesh.push_back(mat::vec3((float*)vptr));
//			}
//
//			return;
//		}
//	}
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
