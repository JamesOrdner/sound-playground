#include "Engine.h"
#include "UScene.h"
#include "UObject.h"
#include "../Audio/AudioEngine.h"
#include "../Graphics/GraphicsSystem.h"
#include "../Graphics/GraphicsScene.h"
#include "../Graphics/GraphicsObject.h"
#include "../UI/UIManager.h"
#include "EInput.h"
#include <SDL.h>

Engine& Engine::instance()
{
	static Engine instance;
	return instance;
}

Engine::Engine() :
	lastFrameTime(0.f),
	input(new EInput),
	graphicsSystem(new GraphicsSystem),
	uiManager(new UIManager),
	audioEngine(new AudioEngine)

{
	input->uiManager = uiManager.get();

	if (init()) {
		setupInitialScene();
		audioEngine->start();
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

bool Engine::init()
{
	// Initialize SDL
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
		printf("SDL could not initialize! SDL Error: %s\n", SDL_GetError());
		return false;
	}

	if (!graphicsSystem->init()) {
		printf("Warning: Graphics system failed to initialize!\n");
		return false;
	}

	if (!audioEngine->init()) {
		printf("Warning: AudioEngine failed to initialize!\n");
		return false;
	}

	return true;
}

void Engine::deinit()
{
	bInitialized = false;
	scenes.clear();
	audioEngine->deinit();
	graphicsSystem->deinit();
	SDL_Quit();
}

void Engine::setupInitialScene()
{
	auto* uscene = scenes.emplace_back(std::make_unique<UScene>()).get();
	auto* graphicsScene = graphicsSystem->createSystemScene<GraphicsScene>(uscene);

	auto* uobject = uscene->createUniversalObject<UObject>();
	auto* graphicsObject = graphicsScene->createSystemObject<GraphicsObject>(uobject);
	graphicsObject->setMesh("res/speaker_small.glb");
}

AudioEngine& Engine::audio()
{
	return *audioEngine;
}

void Engine::run()
{
	if (!bInitialized) return;
	bool quit = false;
	Uint32 sdlTime = SDL_GetTicks();
	while (!quit) {
		// input
		SDL_Event sdlEvent;
		while (SDL_PollEvent(&sdlEvent)) {
			if (sdlEvent.type == SDL_QUIT) {
				quit = true;
				break;
			}
			else {
				input->handleInput(sdlEvent);
			}
		}

		// tick
		Uint32 newSdlTime = SDL_GetTicks();
		lastFrameTime = static_cast<float>(newSdlTime - sdlTime) * 0.001f;
		sdlTime = newSdlTime;
		//m_world->tick(lastFrameTime);
		//uiManager->tick(lastFrameTime);

		graphicsSystem->execute(lastFrameTime);

		// sync changes across systems
		StateManager::instance().notifyObservers();
	}
}

EModel* Engine::raycastScreen(int x, int y) {
	mat::vec3 hitLoc;
	return raycastScreen(x, y, hitLoc);
}

EModel* Engine::raycastScreen(
	int x,
	int y,
	mat::vec3& hitLoc,
	const std::unordered_set<EObject*>& ignore)
{
	return nullptr;

	//mat::vec4 rayEndScreen{
	//	static_cast<float>(x - 1280 / 2) / (1280 / 2),
	//	static_cast<float>(720 / 2 - y) / (720 / 2),
	//	1.f,
	//	1.f };
	//mat::vec4 rayEndWorld(renderer->screenToWorldMatrix() * rayEndScreen);
	//mat::vec3 rayEndWorldNormalized = mat::vec3(rayEndWorld) / rayEndWorld.w;

	//const mat::vec3& rayStartWorld = m_world->worldCamera()->cameraPosition();

	//return m_world->raycast(rayStartWorld, rayEndWorldNormalized - rayStartWorld, hitLoc, ignore);
}
