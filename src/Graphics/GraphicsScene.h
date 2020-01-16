#pragma once

#include "../Engine/SystemSceneInterface.h"
#include <list>
#include <memory>

class GraphicsScene : public SystemSceneInterface
{
public:

	GraphicsScene();

	~GraphicsScene();

	class CameraGraphicsObject* activeCamera;

private:

	std::list<std::unique_ptr<class GraphicsObject>> graphicsObjects;

	SystemObjectInterface* addSystemObject(SystemObjectInterface* object) override;
};
