#pragma once

#include "../Graphics/Matrix.h"
#include <memory>
#include <unordered_set>

// Forward declarations
class UScene;
class AudioEngine;
class UIManager;
class EObject;
class EModel;
class EInput;
class SystemInterface;

class Engine
{
public:

	// Returns the audio engine object
	AudioEngine& audio();

	// Main runloop
	void run();

	// Return the object at screen coordinates
	EModel* raycastScreen(int x, int y);

	// Return the object at screen coordinates, as well as the hit location
	EModel* raycastScreen(
		int x,
		int y,
		mat::vec3& hitLoc,
		const std::unordered_set<EObject*>& ignore = std::unordered_set<EObject*>());

private:

	Engine();
	~Engine();

	bool init();
	void deinit();

	void setupInitialScene();

	// True only after a successful call to init()
	bool bInitialized;

	// Length in seconds of the last frame
	float lastFrameTime;

	// All loaded universal scenes
	std::list<std::unique_ptr<UScene>> scenes;

	// EInput handles all user input
	std::unique_ptr<EInput> input;

	// Graphics
	std::unique_ptr<SystemInterface> graphicsSystem;

	std::unique_ptr<UIManager> uiManager;

	/** Audio */

	std::unique_ptr<AudioEngine> audioEngine;

public:

	static Engine& instance();

	// Deleted functions prevent singleton duplication
	Engine(Engine const&) = delete;
	void operator=(Engine const&) = delete;
};
