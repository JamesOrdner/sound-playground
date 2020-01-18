#pragma once

#include "../../Util/Matrix.h"
#include <string>
#include <vector>
#include <map>
#include <memory>
#include <list>

class GMesh
{
public:

	GMesh(const std::string& filepath);

	~GMesh();

	// Returns a pointer to a mesh object at the specified filepath.
	static GMesh* getSharedMesh(const std::string& filepath);

	// Return all shared meshes
	static const std::map<std::string, std::unique_ptr<GMesh>>& sharedMeshes();

	// Filepath of this mesh, and the key to this mesh in the `meshes` map
	std::string filepath;

	// Register a GraphicsComponent with this mesh
	void registerWithComponent(class MeshGraphicsObject* component);

	// Disassociate a GraphicsComponent with this mesh, deleting the mesh if no references remain
	void unregisterWithComponent(class MeshGraphicsObject* component);

	// Updates any instance transforms or other data that is out of date.
	// Should be called at the beginning of each frame, before any draw() calls.
	void updateInstanceData();

	// Called from the main render loop, draws all models using this mesh
	void draw();

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

	// An ordered list of pointers to all GraphicsObjects sharing this mesh
	std::list<class MeshGraphicsObject*> registeredObjects;

	/** OpenGL */

	// Perform a full reload of any instance buffers
	void reloadInstanceBuffers();

	std::list<GLPrimitive> primitives;

	// Buffers storing rows of instance transformations
	unsigned int vbo_instanceTransforms;

	// Float buffer. 0 == unselected, 1 == selected.
	unsigned int vbo_selected;
};
