#pragma once

#include "../Engine/SystemSceneInterface.h"
#include <SDL_events.h>
#include <list>
#include <memory>

class InputScene : public SystemSceneInterface
{
public:

	InputScene(const class UScene* uscene);

	~InputScene();

	void handleEvent(const SDL_Event& sdlEvent);

	void tick(float deltaTime);

private:

	std::list<std::unique_ptr<class InputObject>> inputObjects;

	SystemObjectInterface* addSystemObject(SystemObjectInterface* object) override;
};
