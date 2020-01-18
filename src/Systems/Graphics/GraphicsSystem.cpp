#include "GraphicsSystem.h"
#include "GraphicsObject.h"
#include "CameraGraphicsObject.h"
#include "GraphicsScene.h"
#include "Render.h"
#include <SDL.h>

GraphicsSystem::GraphicsSystem() :
	window(nullptr)
{
}

GraphicsSystem::~GraphicsSystem()
{
}

bool GraphicsSystem::init()
{
	render = std::make_unique<Render>();

	// Attributes must be set before the window is created
	render->setAttributes();

	// Create window
	window = SDL_CreateWindow("Sound Playground", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
		1280, 720, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);

	if (!window) {
		printf("Window could not be created! SDL Error: %s\n", SDL_GetError());
		return false;
	}

	if (!render->init(window)) {
		printf("Warning: Render failed to initialize!\n");
		return false;
	}

	return true;
}

void GraphicsSystem::deinit()
{
	graphicsScenes.clear();
	render->deinit();
	SDL_DestroyWindow(window);
	window = nullptr;
	render.reset();
}

void GraphicsSystem::execute(float deltaTime)
{
	for (const auto& scene : graphicsScenes) {
		if (CameraGraphicsObject* camera = scene->activeCamera) {
			// loop here doesn't make sense, but eventually we should render per scene
			render->setCamera(camera->cameraPosition(), camera->cameraForward());
		}
	}

	render->drawMeshes();
	//render->drawUI(*uiManager->root, uiManager->screenBounds);
	render->swap(window);
}

SystemSceneInterface* GraphicsSystem::createSystemScene(const class UScene* uscene)
{
	return graphicsScenes.emplace_back(std::make_unique<GraphicsScene>(this, uscene)).get();
}

SystemSceneInterface* GraphicsSystem::findSystemScene(const UScene* uscene)
{
	for (const auto& scene : graphicsScenes) {
		if (scene->uscene == uscene) return scene.get();
	}
	return nullptr;
}

void GraphicsSystem::screenDimensions(int& x, int& y) const
{
	x = 1280;
	y = 720;
}

const mat::mat4& GraphicsSystem::screenToWorldTransform(const UScene* uscene) const
{
	// TODO: lookup per scene
	return render->screenToWorldMatrix();
}
