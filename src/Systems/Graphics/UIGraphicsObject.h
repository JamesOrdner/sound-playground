#pragma once

#include "GraphicsObject.h"
#include "../../Util/Observer.h"
#include "../../UI/UITypes.h"

class UIGraphicsObject : public GraphicsObject, public ObserverInterface, public SubjectInterface
{
public:

	UIGraphicsObject(const class SystemSceneInterface* scene, const class UObject* uobject);

	UIObjectData uiData;
};
