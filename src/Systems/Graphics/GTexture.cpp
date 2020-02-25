#include "GTexture.h"
// #include <GL/gl3w.h>

// #define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

GTexture::GTexture(std::string texture)
{
//	int width, height, nrChannels;
//	std::string path = "res/textures/" + texture + ".tga";
//	stbi_set_flip_vertically_on_load(true);
//	stbi_uc* data = stbi_load(path.c_str(), &width, &height, &nrChannels, STBI_rgb_alpha);
//	if (!data || nrChannels != 4) printf("ERROR: Failed to load texture %s.\n", texture.c_str());
//
//	size.x = static_cast<float>(width);
//	size.y = static_cast<float>(height);
//
//	glCreateTextures(GL_TEXTURE_2D, 1, &textureID);
//	glTextureParameteri(textureID, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
//	glTextureParameteri(textureID, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
//	glTextureStorage2D(textureID, 1, GL_RGBA8, width, height);
//	glTextureSubImage2D(textureID, 0, 0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, data);
//	stbi_image_free(data);
}

GTexture::~GTexture()
{
//	glDeleteTextures(1, &textureID);
}

unsigned int GTexture::id() const
{
	return textureID;
}

const mat::vec2& GTexture::textureSize() const
{
	return size;
}
