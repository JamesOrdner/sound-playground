#pragma once

#include "../Engine/SystemSceneInterface.h"
#include <SDL_events.h>
#include <list>
#include <vector>
#include <memory>

class InputScene : public SystemSceneInterface
{
public:

	InputScene(const class SystemInterface* system, const class UScene* uscene);

	~InputScene();

	void handleEvent(const SDL_Event& sdlEvent);

	void tick(float deltaTime);

private:

	std::list<std::unique_ptr<class InputObject>> inputObjects;

	std::unique_ptr<class UIManager> uiManager;

	std::vector<class InputObject*> selectedObjects;

	bool bPlacingSelectedObjects;

	SystemObjectInterface* addSystemObject(SystemObjectInterface* object) override;

	void handlePlacingInput(const SDL_Event& sdlEvent);
	void handleObjectManagementInput(const SDL_Event& sdlEvent);
};
