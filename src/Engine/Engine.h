#pragma once

#include "../Audio/AudioEngine.h"
#include "../Graphics/Render.h"
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
	
	// Return the object at screen coordinates, as well as the hit location
	std::shared_ptr<EModel> raycastScreen(int x, int y, mat::vec3& hitLoc);

private:
	Engine();
	~Engine();

	bool init();
	void deinit();

	// True only after a successful call to init()
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

	/** SDL */

	SDL_Window* window;

	/** Graphics */

	Render renderer;

	/** Audio */

	AudioEngine audioEngine;

	////// TEMPORARY DEBUG
	std::shared_ptr<EModel> activeModel;
};
