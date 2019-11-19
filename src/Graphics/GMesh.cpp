#include "GMesh.h"
#include <GL/gl3w.h>

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

GMesh::GMesh(const std::string& file)
{
	TinyGLTF loader;
	Model model;
	std::string err;
	std::string warn;
	loader.LoadBinaryFromFile(&model, &err, &warn, file);

	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	GLuint vbos[4];

	Scene& scene = model.scenes[model.defaultScene];
	Node& node = model.nodes[scene.nodes[0]];
	Mesh& mesh = model.meshes[node.mesh];

	for (size_t i = 0; i < model.bufferViews.size(); ++i) {
		const BufferView& bufferView = model.bufferViews[i];
		Buffer& buffer = model.buffers[bufferView.buffer];

		GLuint vbo;
		glGenBuffers(1, &vbo);
		vbos[i] = vbo;
		glBindBuffer(bufferView.target, vbo);
		glBufferData(bufferView.target, bufferView.byteLength,
			&buffer.data.at(0) + bufferView.byteOffset, GL_STATIC_DRAW);
	}

	Primitive primitive = mesh.primitives[0];
	Accessor indexAccessor = model.accessors[primitive.indices];
	drawMode = primitive.mode;
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

	glBindVertexArray(0);
	glDeleteBuffers(4, vbos);
}

void GMesh::draw()
{
	glBindVertexArray(vao);
	glDrawElements(drawMode, drawCount, drawComponentType, bufferOffset(drawByteOffset));
	glBindVertexArray(0);
}
