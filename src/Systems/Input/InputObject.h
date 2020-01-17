#pragma once

#include "../SystemObjectInterface.h"
#include <SDL_events.h>

class InputObject : public SystemObjectInterface
{
public:

	InputObject(const class UObject* uobject);

	virtual ~InputObject();

	virtual void handleEvent(const SDL_Event& sdlEvent);

	virtual void tick(float deltaTime);
};
