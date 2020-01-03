#pragma once

#include "Matrix.h"
#include <string>
#include <list>
#include <map>
#include <memory>

// Forward declarations
struct SDL_Window;
struct UIObject;
class GProgram;
class GMesh;
class GTexture;

class Render
{
public:

	Render();
	~Render();

	// OpenGL attributes require setting before the creation of the SDL_Window
	static void setAttributes();

	// Initialize renderer for use with the SDL_Window
	bool init(SDL_Window* window);

	// Deinitialize memory and delete the OpenGL context
	void deinit();

	// Set the camera position and focus, which sets the projection matrix
	void setCamera(SDL_Window* window, const mat::vec3& position, const mat::vec3& focus);

	// Draw a frame
	void draw(const std::map<std::string, std::weak_ptr<GMesh>>& meshes);

	// Draws the UI on the existing frame
	void drawUI(SDL_Window* window, const UIObject& rootObject);

	// Swaps the backbuffer to the window
	void show(SDL_Window* window);

	// Returns the inverse projection view matrix
	const mat::mat4& screenToWorldMatrix();

private:

	// Pointer to the OpenGL context
	void* glContext;

	// Pointer to the main shader program
	std::unique_ptr<GProgram> programMain;

	// Pointer to the shadow shader program
	std::unique_ptr<GProgram> programShadow;

	unsigned int shadowFBO;
	unsigned int shadowTexture;

	// Pointer to the UI shader program
	std::unique_ptr<GProgram> programUI;

	unsigned int uiVAO;

	// Pointer to the UI texture sheet
	std::unique_ptr<GTexture> uiTexture;

	// Transforms screen space to world space
	mat::mat4 invProjectionViewMatrix;

	// Setup shadow program and buffers. Returns success
	bool initShadow();

	// Setup UI program and buffers. Returns success
	bool initUI();

	// Calculate the projection view matrix
	mat::mat4 projectionViewMatrix(SDL_Window* window);

	void drawUIRecursive(
		const UIObject& object,
		const mat::vec2& parentCoords,
		float parentScale,
		const mat::vec2& screenBounds);
};

