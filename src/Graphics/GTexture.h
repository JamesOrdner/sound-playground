#pragma once

#include "Matrix.h"
#include <string>

class GTexture
{
public:

	// Provide the filename of the texture, without extension or path. Assumes tga format.
	GTexture(std::string texture);

	~GTexture();

	// Returns the opengl texture id
	unsigned int id() const;

	// Returns the size of the texture, in pixels
	const mat::vec2& textureSize() const;

private:

	unsigned int textureID;

	mat::vec2 size;
};
