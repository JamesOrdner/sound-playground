#include "Engine.h"
#include "UScene.h"
#include "UObject.h"
#include "../Managers/StateManager.h"
#include "../Managers/EnvironmentManager.h"
#include "../Input/InputSystem.h"
#include "../Graphics/GraphicsSystem.h"
#include <SDL.h>

#include "../Input/InputScene.h"
#include "../Input/InputObject.h"
#include "../Graphics/GraphicsScene.h"
#include "../Graphics/GraphicsObject.h"

Engine::Engine()
{
	if (init()) {
		setupInitialScene();
		bInitialized = true;
	}
	else {
		bInitialized = false;
	}
}

Engine::~Engine()
{
	deinit();
}

Engine& Engine::instance()
{
	static Engine instance;
	return instance;
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
		return false;
	}

	graphicsSystem = std::make_unique<GraphicsSystem>();
	if (!graphicsSystem->init()) {
		printf("Warning: Graphics system failed to initialize!\n");
		return false;
	}

	return true;
}

void Engine::deinit()
{
	bInitialized = false;
	scenes.clear();
	graphicsSystem->deinit();
	inputSystem->deinit();
	SDL_Quit();
}

void Engine::setupInitialScene()
{
	auto* uscene = scenes.emplace_back(std::make_unique<UScene>()).get();
	auto* inputScene = graphicsSystem->createSystemScene<InputScene>(uscene);
	auto* graphicsScene = graphicsSystem->createSystemScene<GraphicsScene>(uscene);

	auto* uobject = uscene->createUniversalObject<UObject>();
	auto* inputObject = inputScene->createSystemObject<InputObject>(uobject);
	auto* graphicsObject = graphicsScene->createSystemObject<GraphicsObject>(uobject);
	graphicsObject->setMesh("res/speaker_small.glb");
}

void Engine::run()
{
	if (!bInitialized) return;

	Uint32 sdlTime = SDL_GetTicks();

	while (!EnvironmentManager::instance().bQuitRequested) {
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
	}
}
