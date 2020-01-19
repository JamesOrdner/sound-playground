#include "Engine.h"
#include "UScene.h"
#include "Loader.h"
#include "../Systems/SystemInterface.h"
#include "../Managers/StateManager.h"
#include "../Managers/EnvironmentManager.h"
#include <SDL.h>

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
	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		printf("SDL could not initialize! SDL Error: %s\n", SDL_GetError());
		return false;
	}

	loader = std::make_unique<Loader>();

	auto systems = loader->createSystems();
	inputSystem = std::move(systems.input);
	graphicsSystem = std::move(systems.graphics);
	physicsSystem = std::move(systems.physics);

	if (!inputSystem->init()) {
		printf("Warning: Input system failed to initialize!\n");
		deinit();
		return false;
	}

	if (!graphicsSystem->init()) {
		printf("Warning: Graphics system failed to initialize!\n");
		deinit();
		return false;
	}

	if (!physicsSystem->init()) {
		printf("Warning: Graphics system failed to initialize!\n");
		deinit();
		return false;
	}

	bInitialized = true;

	setupInitialScene();

	return true;
}

void Engine::deinit()
{
	bInitialized = false;
	scenes.clear();

	loader.reset();

	if (inputSystem) inputSystem->deinit();
	if (graphicsSystem) graphicsSystem->deinit();
	if (physicsSystem) physicsSystem->deinit();
	
	inputSystem.reset();
	graphicsSystem.reset();
	physicsSystem.reset();

	SDL_Quit();
}

void Engine::setupInitialScene()
{
	auto* uscene = scenes.emplace_back(std::make_unique<UScene>(this)).get();
	loader->loadDefaultScene(uscene);
}

void Engine::run()
{
	if (!bInitialized) return;

	Uint32 sdlTime = SDL_GetTicks();
	EnvironmentManager::instance().bQuitRequested = false;

	do {
		// calculate deltaTime
		Uint32 newSdlTime = SDL_GetTicks();
		float deltaTime = static_cast<float>(newSdlTime - sdlTime) * 0.001f;
		sdlTime = newSdlTime;

		// execute input system on the main thread due to SDL_PollEvent()
		inputSystem->execute(deltaTime);

		// execute systems
		physicsSystem->execute(deltaTime);
		graphicsSystem->execute(deltaTime);

		// sync changes across systems
		StateManager::instance().notifyObservers();
		

	} while (!EnvironmentManager::instance().bQuitRequested);
}

LoaderInterface* Engine::loaderInterface() const
{
	return loader.get();
}
