#pragma once

#include "../Audio/AudioEngine.h"

// Forward declarations
struct SDL_Window;
typedef void* SDL_GLContext;
typedef unsigned int GLuint;

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

	SDL_Window* sdlWindow;
	SDL_GLContext glContext;

	GLuint glProgram;
	GLuint vao;

	GLuint indexBuffer;

	// Mark:- Audio
	AudioEngine audioEngine;
};
