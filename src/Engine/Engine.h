#pragma once

#include "../Audio/AudioEngine.h"
#include "../Graphics/GMesh.h"
#include <vector>
#include <memory>

// Forward declarations
struct SDL_Window;

class Engine
{
public:
	static Engine& instance();

	// Deleted functions prevent singleton duplication
	Engine(Engine const&) = delete;
	void operator=(Engine const&) = delete;

	// Main runloop
	void run();

private:
	Engine();
	~Engine();

	bool init();
	bool initGL();
	void deinit();

	void render();

	bool bInitialized;

	std::vector<std::shared_ptr<GMesh>> meshes;

	/** SDL / OpenGL */

	SDL_Window* sdlWindow;
	void* glContext;
	unsigned int glProgram;

	/** Audio */

	AudioEngine audioEngine;
};
