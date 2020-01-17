#include "Engine.h"
#include "UScene.h"
#include "SystemInterface.h"
#include "../Managers/StateManager.h"
#include "../Managers/AssetManager.h"
#include "../Managers/EnvironmentManager.h"
#include <SDL.h>

// Temporary dependencies
#include "UObject.h"
#include "../Input/InputSystem.h"
#include "../Graphics/GraphicsSystem.h"
#include "../Input/InputScene.h"
#include "../Input/CameraInputObject.h"
#include "../Graphics/GraphicsScene.h"
#include "../Graphics/CameraGraphicsObject.h"
#include "../Graphics/MeshGraphicsObject.h"

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

	assetManager = std::make_unique<AssetManager>();
	assetManager->loadAssets();

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

	auto* ucamera = uscene->createUniversalObject();
	auto* inputObject = inputScene->createSystemObject<CameraInputObject>(ucamera);
	auto* camGraphicsObject = graphicsScene->createSystemObject<CameraGraphicsObject>(ucamera);
	graphicsScene->activeCamera = camGraphicsObject;

	AssetDescriptor asset;
	if (assetManager->descriptor("Platform", asset)) {
		for (int x = -2; x <= 2; x++) {
			for (int z = -1; z <= 2; z++) {
				auto* uplatform = uscene->createUniversalObject();
				auto* pGraphicsObject = graphicsScene->createSystemObject<MeshGraphicsObject>(uplatform);
				pGraphicsObject->setMesh(asset.modelPath);
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
