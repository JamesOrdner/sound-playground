#pragma once

#include "Matrix.h"
#include <string>
#include <vector>
#include <list>
#include <memory>

// Forward declarations
class EModel;
namespace tinygltf
{
	struct Model;
	struct Node;
}

class GMesh
{
public:
	GMesh(const std::string& filepath);

	~GMesh();

	// Register a model with this mesh
	void registerModel(const std::shared_ptr<EModel>& model);

	// Unregisters a model from this mesh
	void unregisterModel(const std::shared_ptr<EModel>& model);

	// Called from the main render loop, draws all models using this mesh
	void draw();

	// Returns the ray mesh buffer.
	const std::vector<mat::vec3>& getRayMesh();

private:

	struct GLPrimitive {
		unsigned int vao;
		unsigned int drawMode;
		int drawCount;
		int drawComponentType;
		void* drawByteOffset;
	};

	// Load the simplified mesh used for raycasting
	void loadRayMesh(const tinygltf::Model& model, const tinygltf::Node& node);

	// An ordered list of weak pointers to all owning models sharing this mesh
	std::list<std::weak_ptr<EModel>> models;

	// The raycasting mesh. Each three vertices forms a triangle.
	std::vector<mat::vec3> rayMeshBuffer;

	/** OpenGL */

	// Perform a full reload of any instance buffers
	void reloadInstanceBuffers();

	// Called each frame, only updates buffers of modified models
	void updateInstanceBuffers();

	std::list<GLPrimitive> primitives;

	// Buffer storing instance positions
	unsigned int vbo_position;

	// Buffer storing instance scales
	unsigned int vbo_scale;
};
