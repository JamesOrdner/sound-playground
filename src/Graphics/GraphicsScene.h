#pragma once

#include "../Engine/SystemSceneInterface.h"
#include <list>
#include <memory>

class GraphicsScene : public SystemSceneInterface
{
public:

	~GraphicsScene();

private:

	std::list<std::unique_ptr<class GraphicsObject>> graphicsObjects;

	SystemObjectInterface* addSystemObject(SystemObjectInterface* object, const UObject* uobject) override;
};
