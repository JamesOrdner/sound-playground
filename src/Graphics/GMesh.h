#pragma once

#include "Matrix.h"
#include <string>
#include <list>
#include <memory>

// Forward declarations
class EModel;

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

private:

	// An ordered list of weak pointers to all owning models sharing this mesh
	std::list<std::weak_ptr<EModel>> models;

	/** OpenGL */

	// Perform a full reload of any instance buffers
	void reloadInstanceBuffers();

	// Called each frame, only updates buffers of modified models
	void updateInstanceBuffers();

	// Buffer storing instance positions
	unsigned int vbo_position;

	// Buffer storing instance scales
	unsigned int vbo_scale;

	unsigned int vao;
	int drawCount;
	int drawComponentType;
	size_t drawByteOffset;
};
