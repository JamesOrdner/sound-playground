#pragma once

#include "../SystemSceneInterface.h"
#include "../../Util/Observer.h"
#include <vector>
#include <list>
#include <memory>

class GraphicsScene : public SystemSceneInterface, public ObserverInterface
{
public:

	GraphicsScene(const class SystemInterface* system, const class UScene* uscene, class VulkanScene* vulkanScene, class VulkanUI* vulkanUI);

	~GraphicsScene();

	const mat::mat4& screenToWorldMatrix() const { return invViewProjMatrix; }

	void deleteSystemObject(const class UObject* uobject) override;

	void draw(class VulkanInstance* vulkan);

	class CameraGraphicsObject* activeCamera;

private:

	class VulkanScene* const vulkanScene;
	class VulkanUI* const vulkanUI;

	SystemObjectInterface* addSystemObject(SystemObjectInterface* object) override;

	std::list<std::unique_ptr<class GraphicsObject>> graphicsObjects;

	std::vector<class UIGraphicsObject*> uiObjects;

	mat::mat4 invViewProjMatrix;
};
