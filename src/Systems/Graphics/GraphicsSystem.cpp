#include "GraphicsSystem.h"
#include "GraphicsObject.h"
#include "GraphicsScene.h"
#include "Vulkan/VulkanInstance.h"
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
	// Create window
	window = SDL_CreateWindow(
		"Sound Playground",
		SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
		1280, 720,
		SDL_WINDOW_VULKAN | SDL_WINDOW_ALLOW_HIGHDPI
	);
	
	if (!window) {
		printf("Window could not be created! SDL Error: %s\n", SDL_GetError());
		return false;
	}
	
	try {
		vulkan = std::make_unique<VulkanInstance>(window);
	} catch(std::exception& e) {
		printf("Vulkan error! %s\n", e.what());
		return false;
	}
	
	return true;
}

void GraphicsSystem::deinit()
{
	graphicsScenes.clear();
	vulkan.reset();
	SDL_DestroyWindow(window);
	window = nullptr;
}

void GraphicsSystem::execute(float deltaTime)
{
	vulkan->beginFrame();
	for (const auto& scene : graphicsScenes) scene->draw(vulkan.get());
	vulkan->endFrameAndPresent();
}

SystemSceneInterface* GraphicsSystem::createSystemScene(const class UScene* uscene)
{
	auto* vulkanScene = vulkan->createScene();
	auto* vulkanUI = vulkan->createUI();
	return graphicsScenes.emplace_back(std::make_unique<GraphicsScene>(this, uscene, vulkanScene, vulkanUI)).get();
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
	for (const auto& scene : graphicsScenes) {
		if (scene->uscene == uscene) return scene->screenToWorldMatrix();
	}

	throw std::runtime_error("Invalid UScene passed to GraphicsSystem::screenToWorldTransform.");
}
