#pragma once

#include "../SystemSceneInterface.h"
#include "../../Util/Observer.h"
#include <vector>
#include <list>
#include <memory>

class GraphicsScene : public SystemSceneInterface, public ObserverInterface
{
public:

	GraphicsScene(const class SystemInterface* system, const class UScene* uscene);

	~GraphicsScene();

	void drawScene(class Render* render);

	class CameraGraphicsObject* activeCamera;

private:

	SystemObjectInterface* addSystemObject(SystemObjectInterface* object) override;

	std::list<std::unique_ptr<class GraphicsObject>> graphicsObjects;

	std::vector<class UIGraphicsObject*> uiObjects;
};
