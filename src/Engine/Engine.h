#pragma once

#include "../Audio/AudioEngine.h"
#include "../Graphics/GMesh.h"
#include "EModel.h"
#include <map>
#include <list>
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

	void registerModel(const std::shared_ptr<EModel>& model);

	// Returns a pointer to an existing mesh at the filepath, or creates a new one
	std::shared_ptr<GMesh> makeMesh(const std::string& filepath);

private:
	Engine();
	~Engine();

	bool init();
	bool initGL();
	void deinit();

	void render();

	bool bInitialized;

	// Stores all registered models, sorted alphabetically by mesh path
	std::list<std::shared_ptr<EModel>> models;

	// Stores pointers to all loaded meshes, indexed by path
	std::map<std::string, std::weak_ptr<GMesh>> meshes;

	/** SDL / OpenGL */

	SDL_Window* sdlWindow;
	void* glContext;
	unsigned int glProgram;

	/** Audio */

	AudioEngine audioEngine;
};
