#include "Engine.h"
#include "../Audio/AudioEngine.h"
#include "../Graphics/Render.h"
#include "../Graphics/Matrix.h"
#include "../Graphics/GMesh.h"
#include "EModel.h"
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
	audioEngine(new AudioEngine)

{
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
		SDL_Event event;
		while (SDL_PollEvent(&event)) {
			int x, y;
			switch (event.type) {
			case SDL_QUIT:
				quit = true;
				break;
			case SDL_MOUSEBUTTONDOWN:
				SDL_GetMouseState(&x, &y);
				if (auto hitObject = raycastScreen(x, y)) {
					const mat::vec3& loc = hitObject->position();
					printf("%f %f\n", loc.x, loc.z);
				}
				break;
			case SDL_MOUSEMOTION:
				SDL_GetMouseState(&x, &y);
				mat::vec3 hitLoc;
				if (EModel* hitObject = raycastScreen(x, y, hitLoc)) {
					activeModel->setPosition(hitLoc);
				}
			}

			if (event.type != SDL_QUIT) input->handleInput(event);
		}

		m_world->tick(lastFrameTime);
		renderer->draw(window, meshes);
		
		Uint32 newSdlTime = SDL_GetTicks();
		lastFrameTime = static_cast<float>(newSdlTime - sdlTime) * 0.001f;
		sdlTime = newSdlTime;
	}
}

void Engine::registerModel(const std::shared_ptr<EModel>& model)
{
	// Assign mesh to model and save model reference
	auto mesh = makeMesh(model->getFilepath());
	model->registerWithMesh(mesh); // transfer mesh ownership to model
	mesh->registerModel(model.get()); // add weak pointer to model in mesh model list

	activeModel = model;
}

void Engine::unregisterModel(const std::shared_ptr<EModel>& model)
{
	model->getMesh()->unregisterModel(model.get());
	model->unregister();

	// Remove mesh from map if no more models reference this mesh
	std::string path = model->getFilepath();
	auto meshRef = meshes[path];
	if (meshRef.expired()) meshes.erase(path);
}

std::shared_ptr<GMesh> Engine::makeMesh(const std::string& filepath)
{
	if (auto existing = meshes[filepath].lock()) {
		return existing;
	}
	else {
		auto newMesh = std::make_shared<GMesh>(filepath);
		meshes[filepath] = newMesh;
		return newMesh;
	}
}

EModel* Engine::raycastScreen(int x, int y) {
	mat::vec3 hitLoc;
	return raycastScreen(x, y, hitLoc);
}

EModel* Engine::raycastScreen(int x, int y, mat::vec3& hitLoc)
{
	using namespace mat;

	int width, height;
	SDL_GL_GetDrawableSize(window, &width, &height);

	vec4 screen_orig{
		static_cast<float>(x - width / 2) / (width / 2),
		static_cast<float>(height / 2 - y) / (height / 2),
		-1.f,
		1.f
	};
	vec3 world_orig(renderer->screenToWorldMatrix()* screen_orig);

	vec4 screen_dir{ 0.f, 0.f, 1.f, 0.f };
	vec3 world_dir(renderer->screenToWorldMatrix()* screen_dir);

	return m_world->raycast(world_orig, world_dir, hitLoc);
}
