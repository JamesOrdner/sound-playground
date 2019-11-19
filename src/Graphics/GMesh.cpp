#include "GMesh.h"
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

GMesh::GMesh(const std::string& filepath) : modelMatrix(mat::mat4::Identity()), scale(1)
{
	TinyGLTF loader;
	Model model;
	std::string err;
	std::string warn;
	loader.LoadBinaryFromFile(&model, &err, &warn, filepath);

	std::vector<GLuint> vbos;

	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	Scene& scene = model.scenes[model.defaultScene];
	for (const Node& node : model.nodes) {
		for (const BufferView& bufferView : model.bufferViews) {
			Buffer& buffer = model.buffers[bufferView.buffer];

			GLuint vbo;
			glGenBuffers(1, &vbo);
			vbos.push_back(vbo);
			glBindBuffer(bufferView.target, vbo);
			glBufferData(bufferView.target, bufferView.byteLength,
				&buffer.data.at(0) + bufferView.byteOffset, GL_STATIC_DRAW);
		}

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

	glBindVertexArray(0);

	for (GLuint vbo : vbos) glDeleteBuffers(1, &vbo);
}

void GMesh::draw(unsigned int modelMatrixID)
{
	glBindVertexArray(vao);
	glUniformMatrix4fv(modelMatrixID, 1, true, *modelMatrix.data);
	glDrawElements(GL_TRIANGLES, drawCount, drawComponentType, bufferOffset(drawByteOffset));
	glBindVertexArray(0);
}

void GMesh::setLocation(const mat::vec3& location)
{
	this->location = location;
	modelMatrix = mat::transform(location, scale);
}

const mat::vec3& GMesh::getLocation()
{
	return location;
}

void GMesh::setScale(float scale)
{
	this->scale = mat::vec3(scale);
	modelMatrix = mat::transform(location, this->scale);
}

void GMesh::setScale(const mat::vec3& scale)
{
	this->scale = scale;
	modelMatrix = mat::transform(location, scale);
}

const mat::vec3& GMesh::getScale()
{
	return scale;
}
