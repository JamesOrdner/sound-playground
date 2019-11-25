#pragma once

#include "../Audio/AudioEngine.h"
#include "EWorld.h"
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

	// Returns the world object
	EWorld& world();

	// Returns the audio engine object
	AudioEngine& audio();

	// Main runloop
	void run();

	// Return the object at screen coordinates
	std::shared_ptr<EModel> raycastScreen(int x, int y);

private:
	Engine();
	~Engine();

	bool init();
	bool initGL();
	void deinit();

	void render();

	bool bInitialized;

	// Register a model with the engine for rendering. Does not check for double registration.
	void registerModel(const std::shared_ptr<EModel>& model);
	friend void EWorld::addObject(const std::shared_ptr<EObject>& object);

	// Remove a model from the rendering pipeline
	void unregisterModel(const std::shared_ptr<EModel>& model);
	friend void EWorld::removeObject(const std::shared_ptr<EObject>& object);

	// Returns a pointer to an existing mesh at the filepath, or creates a new one
	std::shared_ptr<GMesh> makeMesh(const std::string& filepath);

	// The world, containing all objects
	std::unique_ptr<EWorld> _world;

	// Stores pointers to all loaded meshes, indexed by path
	std::map<std::string, std::weak_ptr<GMesh>> meshes;

	/** SDL / OpenGL */

	SDL_Window* sdlWindow;
	void* glContext;
	unsigned int glProgram;

	int screenWidth, screenHeight;

	// Transforms world space to screen space
	mat::mat4 projectionViewMatrix;

	// Transforms screen space to world space
	mat::mat4 invProjectionViewMatrix;

	/** Audio */

	AudioEngine audioEngine;
};
