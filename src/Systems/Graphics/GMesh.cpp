#include "GMesh.h"
#include "MeshGraphicsObject.h"

#include <GL/gl3w.h>
#include <vector>

#pragma warning(push, 0)
#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#define TINYGLTF_USE_CPP14
#include <tiny_gltf.h>
#pragma warning(pop)

using namespace tinygltf;

inline void* bufferOffset(size_t offset) {
	return (char*)nullptr + offset;
}

inline void makeBuffer(GLuint& vbo, GLenum target, GLsizeiptr byteLen, const void* data) {
	glGenBuffers(1, &vbo);
	glBindBuffer(target, vbo);
	glBufferData(target, byteLen, data, GL_STATIC_DRAW);
}

GMesh::GMesh(const std::string& filepath)
{
	this->filepath = filepath;

	TinyGLTF loader;
	Model model;
	std::string err;
	std::string warn;
	loader.LoadBinaryFromFile(&model, &err, &warn, filepath);

	glGenBuffers(1, &vbo_instanceTransforms);
	glGenBuffers(1, &vbo_selected);

	for (const Node& node : model.nodes) {
		if (node.name == "_ray") {
			loadRayMesh(model, node);
			continue;
		}

		for (const Primitive& primitive : model.meshes[node.mesh].primitives) {
			// Store render-time data for primitive
			Accessor indexAccessor = model.accessors[primitive.indices];
			GLPrimitive primitiveData;
			glGenVertexArrays(1, &primitiveData.vao);
			glBindVertexArray(primitiveData.vao);
			primitiveData.drawMode = primitive.mode;
			primitiveData.drawCount = static_cast<int>(indexAccessor.count);
			primitiveData.drawComponentType = indexAccessor.componentType;
			primitiveData.drawByteOffset = bufferOffset(indexAccessor.byteOffset);
			primitives.push_back(primitiveData);

			std::vector<unsigned int> vbos;

			// Element array buffer
			const BufferView& indexBufferView = model.bufferViews[indexAccessor.bufferView];
			const Buffer& indexBuffer = model.buffers[indexBufferView.buffer];
			GLuint vbo;
			makeBuffer(vbo, indexBufferView.target, indexBufferView.byteLength,
				&indexBuffer.data.at(0) + indexBufferView.byteOffset);
			vbos.push_back(vbo);

			// Associate created buffers with proper indices in the vao
			for (auto& attrib : primitive.attributes) {
				const Accessor& accessor = model.accessors[attrib.second];
				const BufferView& bufferView = model.bufferViews[accessor.bufferView];
				const Buffer& buffer = model.buffers[bufferView.buffer];
				GLuint vbo;
				makeBuffer(vbo, bufferView.target, bufferView.byteLength,
					&buffer.data.at(0) + bufferView.byteOffset);
				vbos.push_back(vbo);

				int byteStride = accessor.ByteStride(model.bufferViews[accessor.bufferView]);

				int size = 1;
				if (accessor.type != TINYGLTF_TYPE_SCALAR) {
					size = accessor.type;
				}

				int vaa = -1;
				if (attrib.first == "POSITION") vaa = 0;
				if (attrib.first == "NORMAL") vaa = 1;
				if (attrib.first == "TEXCOORD_0") vaa = 2;
				glEnableVertexAttribArray(vaa);
				glVertexAttribPointer(vaa, size, accessor.componentType,
					accessor.normalized ? GL_TRUE : GL_FALSE,
					byteStride, bufferOffset(accessor.byteOffset));
			}

			// Associate created buffers with proper indices in the vao
			for (auto& attrib : primitive.attributes) {
				const Accessor& accessor = model.accessors[attrib.second];
				const BufferView& bufferView = model.bufferViews[accessor.bufferView];
				const Buffer& buffer = model.buffers[bufferView.buffer];
				GLuint vbo;
				glGenBuffers(1, &vbo);
				glBindBuffer(bufferView.target, vbo);
				glBufferData(bufferView.target, bufferView.byteLength,
					&buffer.data.at(0) + bufferView.byteOffset, GL_STATIC_DRAW);
				vbos.push_back(vbo);

				int size = 1;
				if (accessor.type != TINYGLTF_TYPE_SCALAR) {
					size = accessor.type;
				}

				int vaa = -1;
				if (attrib.first == "POSITION") vaa = 0;
				if (attrib.first == "NORMAL") vaa = 1;
				if (attrib.first == "TEXCOORD_0") vaa = 2;
				glEnableVertexAttribArray(vaa);
				glVertexAttribPointer(vaa, size, accessor.componentType,
					accessor.normalized ? GL_TRUE : GL_FALSE,
					accessor.ByteStride(bufferView), bufferOffset(accessor.byteOffset));
			}

			glBindBuffer(GL_ARRAY_BUFFER, vbo_instanceTransforms);
			for (int i = 0; i < 4; i++) {
				glEnableVertexAttribArray(3 + i);
				glVertexAttribPointer(
					3 + i,
					4,
					GL_FLOAT,
					GL_FALSE,
					sizeof(mat::mat4),
					(void*)(sizeof(float) * 4 * i));
				glVertexAttribDivisor(3 + i, 1);
			}

			glBindBuffer(GL_ARRAY_BUFFER, vbo_selected);
			glEnableVertexAttribArray(7);
			glVertexAttribPointer(7, 1, GL_FLOAT, GL_FALSE, sizeof(float), nullptr);
			glVertexAttribDivisor(7, 1);

			glBindBuffer(GL_ARRAY_BUFFER, 0);
			glBindVertexArray(0);
			glDeleteBuffers(static_cast<GLsizei>(vbos.size()), &vbos[0]);
		}
	}
}

void GMesh::loadRayMesh(const Model& model, const Node& node)
{
	// There should be only one primitive
	const Primitive& primitive = model.meshes[node.mesh].primitives[0];

	// Index array buffer
	Accessor idx_accessor = model.accessors[primitive.indices];
	const BufferView& idx_bufferView = model.bufferViews[idx_accessor.bufferView];
	const Buffer& idx_buffer = model.buffers[idx_bufferView.buffer];

	for (auto& attrib : primitive.attributes) {
		if (attrib.first != "POSITION") continue;

		// Vertex buffer
		const Accessor& vert_accessor = model.accessors[attrib.second];
		const BufferView& vert_bufferView = model.bufferViews[vert_accessor.bufferView];
		const Buffer& vert_buffer = model.buffers[vert_bufferView.buffer];

		assert(primitive.mode == GL_TRIANGLES);
		assert(idx_accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT);
		assert(vert_accessor.type == TINYGLTF_TYPE_VEC3);
		assert(vert_accessor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT);

		rayMeshBuffer.reserve(idx_accessor.count);

		int idx_stride = idx_accessor.ByteStride(idx_bufferView);
		int vert_stride = vert_accessor.ByteStride(vert_bufferView);
		for (size_t i = 0; i < idx_accessor.count; i++) {
			const unsigned char* iptr = &idx_buffer.data[0] + idx_bufferView.byteOffset + idx_stride * i;
			int v_idx = static_cast<int>(*((unsigned short*)iptr));
			const unsigned char* vptr = &vert_buffer.data[0] + vert_bufferView.byteOffset + vert_stride * v_idx;
			rayMeshBuffer.push_back(mat::vec3((float*)vptr));
		}

		return;
	}
}

GMesh::~GMesh()
{
	glDeleteBuffers(1, &vbo_selected);
	glDeleteBuffers(1, &vbo_instanceTransforms);
	for (const auto& primitive : primitives) {
		glDeleteVertexArrays(1, &primitive.vao);
	}
}

std::map<std::string, std::unique_ptr<GMesh>> GMesh::meshes = std::map<std::string, std::unique_ptr<GMesh>>();

GMesh* GMesh::getSharedMesh(const std::string& filepath)
{
	if (auto* existing = meshes[filepath].get()) {
		return existing;
	}
	else {
		auto& mesh = meshes[filepath] = std::make_unique<GMesh>(filepath);
		return mesh.get();
	}
}

const std::map<std::string, std::unique_ptr<GMesh>>& GMesh::sharedMeshes()
{
	return meshes;
}

void GMesh::registerWithComponent(MeshGraphicsObject* component)
{
	registeredObjects.push_back(component);
	reloadInstanceBuffers();
}

void GMesh::unregisterWithComponent(MeshGraphicsObject* component)
{
	registeredObjects.remove(component);
	if (registeredObjects.empty()) meshes.erase(filepath);
	else reloadInstanceBuffers();
}

void GMesh::reloadInstanceBuffers()
{
	std::vector<mat::mat4> transforms;
	std::vector<float> selections;
	for (MeshGraphicsObject* gobject : registeredObjects) {
		transforms.push_back(t(gobject->componentTransformMatrix()));
		selections.push_back(gobject->isSelected() ? 1.f : 0.f);
		gobject->bDirtyTransform = false;
		gobject->bDirtySelection = false;
	}

	glNamedBufferData(vbo_instanceTransforms, registeredObjects.size() * sizeof(mat::mat4), transforms.data(), GL_STATIC_DRAW);
	glNamedBufferData(vbo_selected, registeredObjects.size() * sizeof(float), selections.data(), GL_STATIC_DRAW);
}

void GMesh::updateInstanceData()
{
	// Check for modified model transforms
	size_t i = 0;
	for (MeshGraphicsObject* gobject : registeredObjects) {
		if (gobject->bDirtyTransform) {
			mat::mat4 transform = t(gobject->componentTransformMatrix());
			glNamedBufferSubData(vbo_instanceTransforms, i * sizeof(mat::mat4), sizeof(mat::mat4), transform.data);
			gobject->bDirtyTransform = false;
		}
		if (gobject->bDirtySelection) {
			float selected = gobject->isSelected() ? 1.f : 0.f;
			glNamedBufferSubData(vbo_selected, i * sizeof(float), sizeof(float), &selected);
			gobject->bDirtySelection = false;
		}
		i++;
	}
}

void GMesh::draw()
{
	for (const auto& primitive : primitives) {
		glBindVertexArray(primitive.vao);
		glDrawElementsInstanced(
			primitive.drawMode,
			primitive.drawCount,
			primitive.drawComponentType,
			primitive.drawByteOffset,
			static_cast<GLsizei>(registeredObjects.size()));
	}
	glBindVertexArray(0);
}

const std::vector<mat::vec3>& GMesh::getRayMesh()
{
	return rayMeshBuffer;
}
