#pragma once

#include "Matrix.h"
#include <string>
#include <vector>
#include <map>
#include <memory>
#include <list>

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

	// Returns a pointer to a mesh object at the specified filepath.
	static GMesh* getSharedMesh(const std::string& filepath);

	// Return all shared meshes
	static const std::map<std::string, std::unique_ptr<GMesh>>& sharedMeshes();

	// Register a model with this mesh
	void registerModel(EModel* model);

	// Updates any instance transforms or other data that is out of date.
	// Should be called at the beginning of each frame, before any draw() calls.
	void updateInstanceData();

	// Called from the main render loop, draws all models using this mesh
	void draw();

	// Returns the ray mesh buffer.
	const std::vector<mat::vec3>& getRayMesh();

private:

	// Stores pointers to all loaded meshes, indexed by path
	static std::map<std::string, std::unique_ptr<GMesh>> meshes;

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
	std::list<EModel*> models;

	// The raycasting mesh. Each three vertices forms a triangle.
	std::vector<mat::vec3> rayMeshBuffer;

	/** OpenGL */

	// Perform a full reload of any instance buffers
	void reloadInstanceBuffers();

	std::list<GLPrimitive> primitives;

	// Buffers storing rows of instance transformations
	unsigned int vbo_instanceTransforms;

	// Float buffer. 0 == unselected, 1 == selected.
	unsigned int vbo_selected;
};
