#include "InputObject.h"

InputObject::InputObject(const UObject* uobject) :
	SystemObjectInterface(uobject)
{
}

InputObject::~InputObject()
{
}

void InputObject::handleEvent(const SDL_Event& sdlEvent)
{
}

void InputObject::tick(float deltaTime)
{
}
