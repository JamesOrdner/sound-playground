#include "InputObject.h"

InputObject::InputObject(const SystemSceneInterface* scene, const UObject* uobject) :
	SystemObjectInterface(scene, uobject)
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
