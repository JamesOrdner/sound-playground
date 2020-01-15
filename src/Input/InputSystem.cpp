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
			// handleInput(sdlEvent);
		}
	}
}

SystemSceneInterface* InputSystem::addSystemScene(SystemSceneInterface* scene, const UScene* uscene)
{
	inputScenes.emplace_back(static_cast<InputScene*>(scene));
	return scene;
}
