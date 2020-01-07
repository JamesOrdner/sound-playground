#pragma once

#include "../Graphics/Matrix.h"
#include <memory>

// Forward declarations
struct SDL_Window;
class EWorld;
class AudioEngine;
class UIManager;
class EModel;
class EInput;
class Render;

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

private:

	Engine();
	~Engine();

	bool init();
	void deinit();

	// True only after a successful call to init()
	bool bInitialized;

	// Length in seconds of the last frame
	float lastFrameTime;

	// EInput handles all user input
	std::unique_ptr<EInput> input;

	// The world, containing all objects
	std::unique_ptr<EWorld> m_world;

	/** SDL */

	SDL_Window* window;

	/** Graphics */

	std::unique_ptr<Render> renderer;
	std::unique_ptr<UIManager> uiManager;

	/** Audio */

	std::unique_ptr<AudioEngine> audioEngine;

	/** Utility */

	// Return the object at screen coordinates
	EModel* raycastScreen(int x, int y);

	// Return the object at screen coordinates, as well as the hit location
	EModel* raycastScreen(int x, int y, mat::vec3& hitLoc);
};
