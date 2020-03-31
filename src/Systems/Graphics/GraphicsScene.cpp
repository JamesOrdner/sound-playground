#include "GraphicsScene.h"
#include "GraphicsObject.h"
#include "CameraGraphicsObject.h"
#include "MeshGraphicsObject.h"
#include "UIGraphicsObject.h"
#include "Vulkan/VulkanInstance.h"
#include "Vulkan/VulkanScene.h"
#include "Vulkan/VulkanUI.h"

GraphicsScene::GraphicsScene(const SystemInterface* system, const UScene* uscene, VulkanScene* vulkanScene, VulkanUI* vulkanUI) :
	SystemSceneInterface(system, uscene),
	activeCamera(nullptr),
	vulkanScene(vulkanScene),
	vulkanUI(vulkanUI)
{
}

GraphicsScene::~GraphicsScene()
{
	for (const auto& graphicsObject : graphicsObjects) {
		if (auto* meshObject = dynamic_cast<MeshGraphicsObject*>(graphicsObject.get())) {
			vulkanScene->removeModel(meshObject->model);
		}
	}
	graphicsObjects.clear();
}

SystemObjectInterface* GraphicsScene::addSystemObject(SystemObjectInterface* object)
{
	graphicsObjects.emplace_back(static_cast<GraphicsObject*>(object));
	
	if (auto* meshObject = dynamic_cast<MeshGraphicsObject*>(object)) {
		meshObject->model = vulkanScene->createModel();
	}
	else if (auto* uiObject = dynamic_cast<UIGraphicsObject*>(object)) {
		uiObjects.push_back(uiObject);
		uiObject->vulkanObject = vulkanUI->createUIObject();
		uiObject->dataUpdated();
		registerCallback(
			uiObject,
			EventType::UIDrawOrderUpdated,
			[this](const EventData& data, bool bEventFromParent) {
				vulkanUI->sortDrawOrder();
			}
		);
	}
	
	return object;
}

void GraphicsScene::deleteSystemObject(const UObject* uobject)
{
	for (const auto& graphicsObject : graphicsObjects) {
		if (graphicsObject->uobject == uobject) {
			if (auto* meshObject = dynamic_cast<MeshGraphicsObject*>(graphicsObject.get())) {
				vulkanScene->removeModel(meshObject->model);
			} else if (auto* uiObject = dynamic_cast<UIGraphicsObject*>(graphicsObject.get())) {
				vulkanUI->deleteUIObject(uiObject->vulkanObject);
			}
			graphicsObjects.remove(graphicsObject);
			break;
		}
	}
}

void GraphicsScene::draw(VulkanInstance* vulkan)
{
	if (activeCamera) {
		const auto& pos = activeCamera->cameraPosition();
		const auto& dir = activeCamera->cameraForward();
		const auto viewMatrix = mat::lookAt(pos, pos + dir);
		vulkanScene->setViewMatrix(viewMatrix);
		invViewProjMatrix = mat::inverse(vulkanScene->getProjMatrix() * viewMatrix);
	}
	
	vulkan->draw(vulkanScene, vulkanUI);
}
