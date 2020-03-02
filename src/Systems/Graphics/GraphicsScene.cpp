#include "GraphicsScene.h"
#include "GraphicsObject.h"
#include "CameraGraphicsObject.h"
#include "UIGraphicsObject.h"
#include "Vulkan/VulkanInstance.h"
#include <algorithm>

GraphicsScene::GraphicsScene(const SystemInterface* system, const UScene* uscene, VulkanScene* vulkanScene) :
	SystemSceneInterface(system, uscene),
	activeCamera(nullptr),
	vulkanScene(vulkanScene)
{
}

GraphicsScene::~GraphicsScene()
{
	graphicsObjects.clear();
}

void GraphicsScene::deleteSystemObject(const UObject* uobject)
{
	for (const auto& graphicsObject : graphicsObjects) {
		if (graphicsObject->uobject == uobject) {
			graphicsObjects.remove(graphicsObject);
			break;
		}
	}
}

void GraphicsScene::drawScene(VulkanInstance* vulkan)
{
	// if (activeCamera) vulkan->setCamera(activeCamera->cameraPosition(), activeCamera->cameraForward());
	vulkan->renderScene(vulkanScene);
	// for (auto* uiObject : uiObjects) render->drawUIElement(uiObject->uiData);
}

SystemObjectInterface* GraphicsScene::addSystemObject(SystemObjectInterface* object)
{
	graphicsObjects.emplace_back(static_cast<GraphicsObject*>(object));
	if (auto* uiObject = dynamic_cast<UIGraphicsObject*>(object)) {
		uiObjects.push_back(uiObject);
		registerCallback(
			uiObject,
			EventType::UIDrawOrderUpdated,
			[this](const EventData& data, bool bEventFromParent) {
				std::sort(
					uiObjects.begin(),
					uiObjects.end(),
					[](UIGraphicsObject* l, UIGraphicsObject* r) { return l->uiData.drawOrder < r->uiData.drawOrder; }
				);
			}
		);
	}
	return object;
}
