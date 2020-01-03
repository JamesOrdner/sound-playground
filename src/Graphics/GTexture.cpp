#include "GTexture.h"
#include <GL/gl3w.h>

// #define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

GTexture::GTexture(std::string texture)
{
	int width, height, nrChannels;
	std::string path = "res/textures/" + texture + ".tga";
	stbi_set_flip_vertically_on_load(true);
	unsigned char* data = stbi_load(path.c_str(), &width, &height, &nrChannels, 0);
	if (!data) printf("ERROR: Failed to load texture %s.\n", texture.c_str());

	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_2D, textureID);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
	stbi_image_free(data);
	glBindTexture(GL_TEXTURE_2D, 0);
}

GTexture::~GTexture()
{
	glDeleteTextures(1, &textureID);
}

unsigned int GTexture::id() const
{
	return textureID;
}
