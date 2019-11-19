#pragma once

#include <string>

// Forward declarations
typedef unsigned int GLuint;

class GMesh
{
public:
	GMesh(const std::string& file);
	void draw();

private:
	GLuint vao;

	int drawMode;
	int drawCount;
	int drawComponentType;
	size_t drawByteOffset;
};

