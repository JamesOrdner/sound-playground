#include "Engine.h"
#include "UScene.h"
#include "Loader.h"
#include "SystemInterface.h"
#include "../Managers/ServiceManager.h"
#include "../Managers/StateManager.h"
#include "../Managers/AssetManager.h"
#include "../Managers/EnvironmentManager.h"
#include <SDL.h>

// Temporary dependencies
#include "UObject.h"

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
	auto* inputScene = inputSystem->createSystemScene(uscene);
	auto* graphicsScene = graphicsSystem->createSystemScene(uscene);

	loader->createDefaultCamera(uscene);

	AssetDescriptor asset;
	if (AssetManager::instance().descriptor("Platform", asset)) {
		for (int x = -2; x <= 2; x++) {
			for (int z = -1; z <= 2; z++) {
				auto* uplatform = loader->createObjectFromAsset(asset, uscene);
				uplatform->event(
					EventType::PositionUpdated,
					mat::vec3 { static_cast<float>(x), 0, static_cast<float>(z) - 0.5f });
			}
		}
	}
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

		// sync changes across systems
		StateManager::instance().notifyObservers();

		// by executing the graphics system after syncing changes, we reduce input lag
		graphicsSystem->execute(deltaTime);

	} while (!EnvironmentManager::instance().bQuitRequested);
}

LoaderInterface* Engine::loaderInterface() const
{
	return loader.get();
}
