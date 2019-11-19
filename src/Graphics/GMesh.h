#pragma once

#include <string>
#include "Matrix.h"

class GMesh
{
public:
	GMesh(const std::string& filepath);

	// Name used to identify this mesh
	std::string name;

	// Called from the main render loop, accepting modelMatrixID to load model matrix
	void draw(unsigned int modelMatrixID);

	/** Location setters and getters */

	void setLocation(const mat::vec3& location);
	const mat::vec3& getLocation();

	/** Scale setters and getters */

	void setScale(float scale);
	void setScale(const mat::vec3& scale);
	const mat::vec3& getScale();

private:
	// World space location
	mat::vec3 location;

	// World space scale
	mat::vec3 scale;

	// modelMatrix updates only if location, rotation, or scale are modified
	mat::mat4 modelMatrix;

	/** OpenGL */

	unsigned int vao;
	int drawCount;
	int drawComponentType;
	size_t drawByteOffset;
};
