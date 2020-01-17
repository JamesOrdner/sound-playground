#pragma once

#include "../SystemSceneInterface.h"
#include <list>
#include <memory>

class GraphicsScene : public SystemSceneInterface
{
public:

	GraphicsScene(const class SystemInterface* system, const class UScene* uscene);

	~GraphicsScene();

	class CameraGraphicsObject* activeCamera;

private:

	std::list<std::unique_ptr<class GraphicsObject>> graphicsObjects;

	SystemObjectInterface* addSystemObject(SystemObjectInterface* object) override;
};
