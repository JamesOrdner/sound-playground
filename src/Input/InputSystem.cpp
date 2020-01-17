#include "InputSystem.h"
#include "InputScene.h"
#include "../Managers/EnvironmentManager.h"
#include <SDL_events.h>

InputSystem::InputSystem()
{
}

InputSystem::~InputSystem()
{
}

bool InputSystem::init()
{
	return true;
}

void InputSystem::deinit()
{
}

void InputSystem::execute(float deltaTime)
{
	SDL_Event sdlEvent;
	while (SDL_PeepEvents(&sdlEvent, 1, SDL_GETEVENT, SDL_FIRSTEVENT, SDL_LASTEVENT)) {
		if (sdlEvent.type == SDL_QUIT) {
			EnvironmentManager::instance().bQuitRequested = true;
			break;
		}
		else {
			for (const auto& scene : inputScenes) {
				scene->handleEvent(sdlEvent);
			}
		}
	}

	for (const auto& scene : inputScenes) {
		scene->tick(deltaTime);
	}
}

SystemSceneInterface* InputSystem::createSystemScene(const UScene* uscene)
{
	return inputScenes.emplace_back(std::make_unique<InputScene>(this, uscene)).get();
}

SystemSceneInterface* InputSystem::findSystemScene(const UScene* uscene)
{
	for (const auto& scene : inputScenes) {
		if (scene->uscene == uscene) return scene.get();
	}
	return nullptr;
}
