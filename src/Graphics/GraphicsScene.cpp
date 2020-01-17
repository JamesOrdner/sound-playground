#include "GraphicsScene.h"
#include "GraphicsObject.h"

GraphicsScene::GraphicsScene(const class UScene* uscene) :
	SystemSceneInterface(uscene),
	activeCamera(nullptr)
{
}

GraphicsScene::~GraphicsScene()
{
	graphicsObjects.clear();
}

SystemObjectInterface* GraphicsScene::addSystemObject(SystemObjectInterface* object)
{
	graphicsObjects.emplace_back(static_cast<GraphicsObject*>(object));
	return object;
}
