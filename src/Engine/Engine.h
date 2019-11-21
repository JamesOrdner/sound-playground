#pragma once

#include "../Audio/AudioEngine.h"
#include <string>
#include <map>
#include <list>
#include <memory>

// Forward declarations
struct SDL_Window;
class EModel;
class GMesh;

class Engine
{
public:
	static Engine& instance();

	// Deleted functions prevent singleton duplication
	Engine(Engine const&) = delete;
	void operator=(Engine const&) = delete;

	// Main runloop
	void run();

	// Register a model with the engine for rendering. Does not check for double registration.
	void registerModel(const std::shared_ptr<EModel>& model);

	// Remove a model from the rendering pipeline
	void unregisterModel(const std::shared_ptr<EModel>& model);

private:
	Engine();
	~Engine();

	bool init();
	bool initGL();
	void deinit();

	void render();

	bool bInitialized;

	// Returns a pointer to an existing mesh at the filepath, or creates a new one
	std::shared_ptr<GMesh> makeMesh(const std::string& filepath);

	// Stores all registered models
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
