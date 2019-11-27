#pragma once

#include "Matrix.h"
#include <string>
#include <map>
#include <memory>

// Forward declarations
struct SDL_Window;
class GMesh;

class Render
{
public:

	Render();

	// OpenGL attributes require setting before the creation of the SDL_Window
	static void setAttributes();

	// Initialize renderer for use with the SDL_Window
	bool init(SDL_Window* window);

	// Deinitialize memory and delete the OpenGL context
	void deinit();

	// Draw a frame
	void draw(SDL_Window* window, const std::map<std::string, std::weak_ptr<GMesh>>& meshes);

	// Returns the inverse projection view matrix
	const mat::mat4& screenToWorldMatrix();

private:

	// Create and register any gl buffers. Returns success
	bool initBuffers();

	// Read the contents of a text file at the specified path into a string
	std::string loadTextFile(std::string filepath);

	// Pointer to the OpenGL context
	void* glContext;

	// Global shader program
	unsigned int glProgram;

	// Shadow shader
	unsigned int glShadowProgram;

	unsigned int shadowBuffer;
	unsigned int shadowTexture;

	// Transforms screen space to world space
	mat::mat4 invProjectionViewMatrix;
};

