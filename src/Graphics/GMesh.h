#pragma once

#include <string>
#include "Matrix.h"

class GMesh
{
public:
	GMesh(const std::string& filepath);

	// Returns the path of the mesh file
	std::string meshFilepath();

	// Called from the main render loop, accepting modelMatrixID to load model matrix
	void draw(unsigned int modelMatrixID,  const mat::mat4& modelMatrix);

private:

	std::string filepath;

	/** OpenGL */

	unsigned int vao;
	int drawCount;
	int drawComponentType;
	size_t drawByteOffset;
};
