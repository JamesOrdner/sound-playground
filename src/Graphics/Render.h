#pragma once

#include "../Util/Matrix.h"
#include <memory>

// Forward declarations
struct SDL_Window;
struct UIObject;
class GProgram;
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

	// Deinitialize memory and delete the OpenGL context (safe to call multiple times)
	void deinit();

	// Set the camera position and orientation, which sets the projection matrix
	void setCamera(const mat::vec3& position, const mat::vec3& forward);

	// Draw a frame
	void drawMeshes();

	// Draws the UI on the existing frame, provided the virtual UI screen dimensions
	void drawUI(const UIObject& rootObject, const mat::vec2& virtualScreenBounds);

	// Swaps the backbuffer to the window
	void swap(SDL_Window* window);

	// Returns the inverse projection view matrix
	const mat::mat4& screenToWorldMatrix();

private:

	struct GObjects
	{
		typedef unsigned int GLuint;

		GLuint shadowFBO;
		GLuint shadowTexture;
		GLuint rectVAO;
		GLuint gbuffer;
		GLuint gbufferTextures[3];

		void deinit();
	};

	GObjects gObjects;

	void* glContext;

	// Transforms screen space to world space
	mat::mat4 invProjectionViewMatrix;

	std::unique_ptr<GProgram> mainProgram;

	// rectVAO is used for deferred composite and UI rendering
	bool initRectVAO();

	bool initDeferredPipeline();
	std::unique_ptr<GProgram> gbuffersProgram;
	std::unique_ptr<GProgram> compositeProgram;

	bool initShadow();
	std::unique_ptr<GProgram> shadowProgram;

	bool initUI();
	std::unique_ptr<GProgram> uiProgram;
	std::unique_ptr<GTexture> uiTexture;

	void drawUIRecursive(
		const UIObject& object,
		const mat::vec2& parentCenterAbs,
		const mat::vec2& parentBoundsAbs,
		const mat::vec2& screenBounds);
};
