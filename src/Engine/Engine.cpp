#include "Engine.h"
#include "EWorld.h"
#include "../Audio/AudioEngine.h"
#include "../Graphics/Render.h"
#include "../UI/UIManager.h"
#include "../Objects/EModel.h"
#include "../Objects/ECamera.h"
#include "EInput.h"
#include <SDL.h>

Engine& Engine::instance()
{
	static Engine instance;
	return instance;
}

Engine::Engine() :
	lastFrameTime(0.f),
	m_world(new EWorld),
	input(new EInput),
	renderer(new Render),
	uiManager(new UIManager),
	audioEngine(new AudioEngine)

{
	input->uiManager = uiManager.get();

	if (init()) {
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

	// Attributes must be set before the window is created
	renderer->setAttributes();

	// Create window
	window = SDL_CreateWindow("Sound Playground", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
		1280, 720, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);

	if (!window) {
		printf("Window could not be created! SDL Error: %s\n", SDL_GetError());
		return false;
	}

	if (!renderer->init(window)) {
		printf("Warning: Renderer failed to initialize!\n");
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
	m_world.reset(); // This will deinit all objects
	audioEngine->deinit();
	renderer->deinit();
	SDL_DestroyWindow(window);
	window = nullptr;
	SDL_Quit();
}

EWorld& Engine::world()
{
	return *m_world;
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
		m_world->tick(lastFrameTime);
		uiManager->tick(lastFrameTime);

		// render
		const ECamera* camera = m_world->worldCamera();
		renderer->setCamera(camera->cameraPosition(), camera->cameraFocus());
		renderer->drawMeshes();
		renderer->drawUI(*uiManager->root, uiManager->screenBounds);
		renderer->swap(window);

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
	int width, height;
	SDL_GL_GetDrawableSize(window, &width, &height);

	mat::vec4 rayEndScreen{
		static_cast<float>(x - width / 2) / (width / 2),
		static_cast<float>(height / 2 - y) / (height / 2),
		1.f,
		1.f };
	mat::vec4 rayEndWorld(renderer->screenToWorldMatrix() * rayEndScreen);
	mat::vec3 rayEndWorldNormalized = mat::vec3(rayEndWorld) / rayEndWorld.w;

	const mat::vec3& rayStartWorld = m_world->worldCamera()->cameraPosition();

	return m_world->raycast(rayStartWorld, rayEndWorldNormalized - rayStartWorld, hitLoc, ignore);
}
