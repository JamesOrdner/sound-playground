#include "InputScene.h"
#include "InputObject.h"

InputScene::~InputScene()
{
}

SystemObjectInterface* InputScene::addSystemObject(SystemObjectInterface* object, const UObject* uobject)
{
	inputObjects.emplace_back(static_cast<InputObject*>(object));
	return object;
}
