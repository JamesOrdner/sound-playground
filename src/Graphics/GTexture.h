#pragma once

#include <string>

class GTexture
{
public:

	// Provide the filename of the texture, without extension or path. Assumes tga format.
	GTexture(std::string texture);

	~GTexture();

	// Returns the opengl texture id
	unsigned int id() const;

private:

	unsigned int textureID;
};
