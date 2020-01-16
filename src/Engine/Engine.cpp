#include "Engine.h"
#include "UScene.h"
#include "SystemInterface.h"
#include "../Managers/StateManager.h"
#include "../Managers/EnvironmentManager.h"
#include <SDL.h>

// Temporary dependencies
#include "UObject.h"
#include "../Input/InputSystem.h"
#include "../Graphics/GraphicsSystem.h"
#include "../Input/InputScene.h"
#include "../Input/CameraInputObject.h"
#include "../Graphics/GraphicsScene.h"
#include "../Graphics/GraphicsObject.h"

Engine::Engine() :
	bInitialized(false)
{
}

Engine::~Engine()
{
}

bool Engine::init()
{
	// Initialize SDL
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
		printf("SDL could not initialize! SDL Error: %s\n", SDL_GetError());
		return false;
	}

	inputSystem = std::make_unique<InputSystem>();
	if (!inputSystem->init()) {
		printf("Warning: Input system failed to initialize!\n");
		deinit();
		return false;
	}

	graphicsSystem = std::make_unique<GraphicsSystem>();
	if (!graphicsSystem->init()) {
		printf("Warning: Graphics system failed to initialize!\n");
		deinit();
		return false;
	}

	setupInitialScene();

	bInitialized = true;

	return true;
}

void Engine::deinit()
{
	bInitialized = false;
	scenes.clear();

	if (graphicsSystem) graphicsSystem->deinit();
	if (inputSystem) inputSystem->deinit();
	graphicsSystem.reset();
	inputSystem.reset();

	SDL_Quit();
}

void Engine::setupInitialScene()
{
	auto* uscene = scenes.emplace_back(std::make_unique<UScene>()).get();
	auto* inputScene = inputSystem->createSystemScene<InputScene>(uscene);
	auto* graphicsScene = graphicsSystem->createSystemScene<GraphicsScene>(uscene);

	auto* uspeaker = uscene->createUniversalObject();
	auto* graphicsObject = graphicsScene->createSystemObject<GraphicsObject>(uspeaker);
	graphicsObject->setMesh("res/speaker_small.glb");

	// TEMP
	auto* inputObject = inputScene->createSystemObject<CameraInputObject>(uspeaker);

	//auto* ucamera = uscene->createUniversalObject();
	//auto* inputObject = inputScene->createSystemObject<CameraInputObject>(ucamera);
	//auto* camGraphicsObject = graphicsScene->createSystemObject<GraphicsObject>(ucamera);
}

void Engine::run()
{
	if (!bInitialized) return;

	Uint32 sdlTime = SDL_GetTicks();
	EnvironmentManager::instance().bQuitRequested = false;

	do {
		// pump pending OS/input events to SDL's event queue
		SDL_PumpEvents();

		// calculate deltaTime
		Uint32 newSdlTime = SDL_GetTicks();
		float deltaTime = static_cast<float>(newSdlTime - sdlTime) * 0.001f;
		sdlTime = newSdlTime;

		// execute systems
		inputSystem->execute(deltaTime);
		graphicsSystem->execute(deltaTime);

		// sync changes across systems
		StateManager::instance().notifyObservers();

	} while (!EnvironmentManager::instance().bQuitRequested);
}
