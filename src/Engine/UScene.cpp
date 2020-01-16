#include "UScene.h"
#include "UObject.h"

UScene::~UScene()
{
}

UObject* UScene::createUniversalObject()
{
	return objects.emplace_back(std::make_unique<UObject>()).get();
}
