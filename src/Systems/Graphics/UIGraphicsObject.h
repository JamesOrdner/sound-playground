#pragma once

#include "GraphicsObject.h"
#include "../../Util/Observer.h"
#include "../../UI/UITypes.h"

class UIGraphicsObject : public GraphicsObject, public ObserverInterface, public SubjectInterface
{
public:
	
	UIGraphicsObject(const class SystemSceneInterface* scene, const class UObject* uobject);
	
	/// Called when uiData changes or vulkanObject is set. Updates VulkanUIObject
	void dataUpdated();
	
	UIObjectData uiData;
	
	struct VulkanUIObject* vulkanObject;
};
