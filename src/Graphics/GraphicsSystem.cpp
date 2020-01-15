#include "GraphicsSystem.h"
#include "GraphicsObject.h"
#include "GraphicsScene.h"
#include "Render.h"
#include <SDL.h>

GraphicsSystem::GraphicsSystem() :
	window(nullptr),
	render(new Render)
{
}

GraphicsSystem::~GraphicsSystem()
{
}

bool GraphicsSystem::init()
{
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
	render->deinit();
	SDL_DestroyWindow(window);
	window = nullptr;
}

void GraphicsSystem::execute(float deltaTime)
{
	render->setCamera(mat::vec3{ 2, 0, 0 }, mat::vec3());
	render->drawMeshes();
	//render->drawUI(*uiManager->root, uiManager->screenBounds);
	render->swap(window);
}

SystemSceneInterface* GraphicsSystem::addSystemScene(SystemSceneInterface* scene, const UScene* uscene)
{
	graphicsScenes.emplace_back(static_cast<GraphicsScene*>(scene));
	return scene;
}
