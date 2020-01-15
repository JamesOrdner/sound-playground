#include "UScene.h"
#include "UObject.h"

UScene::~UScene()
{
}

UObject* UScene::addUniversalObject(UObject* object)
{
	objects.emplace_back(object);
	return object;
}
