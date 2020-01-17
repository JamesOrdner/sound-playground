#include "InputScene.h"
#include "InputObject.h"

InputScene::InputScene(const UScene* uscene) :
	SystemSceneInterface(uscene)
{
}

InputScene::~InputScene()
{
}

void InputScene::handleEvent(const SDL_Event& sdlEvent)
{
	for (const auto& object : inputObjects) {
		object->handleEvent(sdlEvent);
	}
}

void InputScene::tick(float deltaTime)
{
	for (const auto& object : inputObjects) {
		object->tick(deltaTime);
	}
}

SystemObjectInterface* InputScene::addSystemObject(SystemObjectInterface* object)
{
	inputObjects.emplace_back(static_cast<InputObject*>(object));
	return object;
}
