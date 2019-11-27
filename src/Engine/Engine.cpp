#include "Engine.h"
#include "../Graphics/Matrix.h"
#include "../Graphics/GMesh.h"
#include "EModel.h"
#include <SDL.h>

Engine& Engine::instance()
{
	static Engine instance;
	return instance;
}

EWorld& Engine::world()
{
	return *_world;
}

AudioEngine& Engine::audio()
{
	return audioEngine;
}

void Engine::run()
{
	if (!bInitialized) return;
	bool quit = false;
	while (!quit) {
		SDL_Event event;
		while (SDL_PollEvent(&event)) {
			switch (event.type) {
			case SDL_QUIT:
				quit = true;
				break;
			case SDL_MOUSEBUTTONDOWN:
				int x, y;
				SDL_GetMouseState(&x, &y);
				if (auto hitObject = raycastScreen(x, y)) {
					const mat::vec3& loc = hitObject->getPosition();
					printf("%f %f\n", loc.x, loc.z);
				}
			}
		}

		renderer.draw(window, meshes);
	}
}

std::shared_ptr<EModel> Engine::raycastScreen(int x, int y)
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
	vec3 world_orig(renderer.screenToWorldMatrix() * screen_orig);

	vec4 screen_dir{ 0.f, 0.f, 1.f, 0.f };
	vec3 world_dir(renderer.screenToWorldMatrix() * screen_dir);

	vec3 hitLoc;
	return _world->raycast(world_orig, world_dir, hitLoc);
}

void Engine::registerModel(const std::shared_ptr<EModel>& model)
{
	// Assign mesh to model and save model reference
	auto mesh = makeMesh(model->getFilepath());
	model->registerWithMesh(mesh); // transfer mesh ownership to model
	mesh->registerModel(model); // add weak pointer to model in mesh model list
}

void Engine::unregisterModel(const std::shared_ptr<EModel>& model)
{
	model->getMesh()->unregisterModel(model);
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

Engine::Engine()
{
	if (init()) {
		_world = std::make_unique<EWorld>();
		audioEngine.start();
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
	renderer.setAttributes();

	// Create window
	window = SDL_CreateWindow("Sound Playground", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
		1280, 720, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);

	if (!window) {
		printf("Window could not be created! SDL Error: %s\n", SDL_GetError());
		return false;
	}

	if (!renderer.init(window)) {
		printf("Warning: Renderer failed to initialize!\n");
		return false;
	}

	if (!audioEngine.init()) {
		printf("Warning: AudioEngine failed to initialize!\n");
		return false;
	}
	
	return true;
}

void Engine::deinit()
{
	bInitialized = false;
	renderer.deinit();
	audioEngine.deinit();
	SDL_DestroyWindow(window);
	window = nullptr;
	SDL_Quit();
}
