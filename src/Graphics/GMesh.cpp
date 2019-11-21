#include "GMesh.h"
#include "../Engine/EModel.h"

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

GMesh::GMesh(const std::string& filepath)
{
	TinyGLTF loader;
	Model model;
	std::string err;
	std::string warn;
	loader.LoadBinaryFromFile(&model, &err, &warn, filepath);

	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	std::vector<unsigned int> vbos;

	Scene& scene = model.scenes[model.defaultScene];
	for (const Node& node : model.nodes) {
		// Create vertex buffers and copy data to device
		for (const BufferView& bufferView : model.bufferViews) {
			Buffer& buffer = model.buffers[bufferView.buffer];

			GLuint vbo;
			glGenBuffers(1, &vbo);
			vbos.push_back(vbo);
			glBindBuffer(bufferView.target, vbo);
			glBufferData(bufferView.target, bufferView.byteLength,
				&buffer.data.at(0) + bufferView.byteOffset, GL_STATIC_DRAW);
		}

		// Associate created buffers with proper indices in the vao
		for (const Primitive& primitive : model.meshes[node.mesh].primitives) {
			Accessor indexAccessor = model.accessors[primitive.indices];
			// drawMode = primitive.mode;
			drawCount = static_cast<int>(indexAccessor.count);
			drawComponentType = indexAccessor.componentType;
			drawByteOffset = indexAccessor.byteOffset;

			for (auto& attrib : primitive.attributes) {
				Accessor accessor = model.accessors[attrib.second];
				int byteStride = accessor.ByteStride(model.bufferViews[accessor.bufferView]);
				glBindBuffer(GL_ARRAY_BUFFER, vbos[accessor.bufferView]);

				int size = 1;
				if (accessor.type != TINYGLTF_TYPE_SCALAR) {
					size = accessor.type;
				}

				int vaa = -1;
				if (attrib.first.compare("POSITION") == 0) vaa = 0;
				if (attrib.first.compare("NORMAL") == 0) vaa = 1;
				if (attrib.first.compare("TEXCOORD_0") == 0) vaa = 2;
				glEnableVertexAttribArray(vaa);
				glVertexAttribPointer(vaa, size, accessor.componentType,
					accessor.normalized ? GL_TRUE : GL_FALSE,
					byteStride, bufferOffset(accessor.byteOffset));
			}
		}
	}

	// Create instance position buffer
	glGenBuffers(1, &vbo_position);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_position);
	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(3);
	glVertexAttribDivisor(3, 1);

	// Create instance scale buffer
	glGenBuffers(1, &vbo_scale);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_scale);
	glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(4);
	glVertexAttribDivisor(4, 1);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	// Won't need to modify these anymore, delete
	glDeleteBuffers(static_cast<GLsizei>(vbos.size()), &vbos[0]);
}

GMesh::~GMesh()
{
	glDeleteBuffers(1, &vbo_position);
	glDeleteBuffers(1, &vbo_scale);
}

void GMesh::registerModel(const std::shared_ptr<EModel>& model)
{
	models.push_back(model);
	reloadInstanceBuffers();
}

void GMesh::unregisterModel(const std::shared_ptr<EModel>& model)
{
	for (auto it = models.cbegin(); it != models.cend(); it++) {
		if (it->lock() == model) {
			models.erase(it);
			reloadInstanceBuffers();
			return;
		}
	}
}

void GMesh::reloadInstanceBuffers()
{
	std::vector<mat::vec3> positions;
	std::vector<mat::vec3> scales;
	for (const auto& model : models) {
		auto modelPtr = model.lock();
		positions.push_back(modelPtr->getPosition());
		scales.push_back(modelPtr->getScale());
		modelPtr->transformUpdated();
	}

	glBindBuffer(GL_ARRAY_BUFFER, vbo_position);
	glBufferData(GL_ARRAY_BUFFER, models.size() * sizeof(mat::vec3), &positions[0], GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindBuffer(GL_ARRAY_BUFFER, vbo_scale);
	glBufferData(GL_ARRAY_BUFFER, models.size() * sizeof(mat::vec3), &scales[0], GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void GMesh::updateInstanceBuffers()
{
	// Check for modified model transforms
	size_t i = 0;
	for (const auto& model : models) {
		auto modelPtr = model.lock();
		if (modelPtr->needsTransformUpdate()) {
			glBindBuffer(GL_ARRAY_BUFFER, vbo_position);
			glBufferSubData(GL_ARRAY_BUFFER, i * sizeof(mat::vec3), sizeof(mat::vec3), modelPtr->getPosition().data);
			
			glBindBuffer(GL_ARRAY_BUFFER, vbo_scale);
			glBufferSubData(GL_ARRAY_BUFFER, i * sizeof(mat::vec3), sizeof(mat::vec3), modelPtr->getScale().data);

			glBindBuffer(GL_ARRAY_BUFFER, 0);
			modelPtr->transformUpdated();
		}
		i++;
	}
}

void GMesh::draw()
{
	updateInstanceBuffers();
	glBindVertexArray(vao);
	glDrawElementsInstanced(
		GL_TRIANGLES, 
		drawCount, 
		drawComponentType, 
		bufferOffset(drawByteOffset), 
		static_cast<GLsizei>(models.size()));
	glBindVertexArray(0);
}
