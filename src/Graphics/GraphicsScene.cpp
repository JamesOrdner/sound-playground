#include "GraphicsScene.h"
#include "GraphicsObject.h"

GraphicsScene::~GraphicsScene()
{
}

SystemObjectInterface* GraphicsScene::addSystemObject(SystemObjectInterface* object, const UObject* uobject)
{
	graphicsObjects.emplace_back(static_cast<GraphicsObject*>(object));
	return object;
}
