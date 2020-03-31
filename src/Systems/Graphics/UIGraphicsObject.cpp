#include "UIGraphicsObject.h"
#include "GraphicsScene.h"
#include "../SystemInterface.h"
#include "../../Managers/AssetManagerInterface.h"
#include "../../Engine/UObject.h"
#include "Vulkan/VulkanUI.h"
#include "Vulkan/VulkanUIObject.h"

UIGraphicsObject::UIGraphicsObject(const SystemSceneInterface* scene, const UObject* uobject) :
	GraphicsObject(scene, uobject),
	vulkanObject(nullptr)
{
	registerCallback(
		uobject,
		EventType::UIPositionUpdated,
		[this](const EventData& data, bool bEventFromParent) {
			uiData.position = std::get<mat::vec2>(data);
			dataUpdated();
		}
	);

	registerCallback(
		uobject,
		EventType::UIBoundsUpdated,
		[this](const EventData& data, bool bEventFromParent) {
			uiData.bounds = std::get<mat::vec2>(data);
			dataUpdated();
		}
	);

	registerCallback(
		uobject,
		EventType::UIDrawOrderUpdated,
		[this](const EventData& data, bool bEventFromParent) {
			uiData.drawOrder = std::get<uint32_t>(data);
			dataUpdated();
		}
	);

	registerCallback(
		uobject,
		EventType::UITextureAssetUpdated,
		[this](const EventData& data, bool bEventFromParent) {
			uiData.textureAsset = std::get<AssetID>(data);
			dataUpdated();
		}
	);

	registerCallback(
		uobject,
		EventType::UITexturePositionUpdated,
		[this](const EventData& data, bool bEventFromParent) {
			uiData.texturePosition = std::get<mat::vec2>(data);
			dataUpdated();
		}
	);

	registerCallback(
		uobject,
		EventType::UITextureBoundsUpdated,
		[this](const EventData& data, bool bEventFromParent) {
			uiData.textureBounds = std::get<mat::vec2>(data);
			dataUpdated();
		}
	);
}

void UIGraphicsObject::dataUpdated()
{
	if (!vulkanObject) return;
	
	// TODO: TEMP
	constexpr mat::vec2 resolution{ 1280.f, 720.f };
	
	vulkanObject->position = uiData.position / resolution * 2.f - 1.f;
	vulkanObject->bounds = uiData.bounds / resolution * 2.f;
	
	vulkanObject->uv_position = uiData.texturePosition / 1024.f;
	vulkanObject->uv_bounds = uiData.textureBounds / 1024.f;
	
	// flip y axis
	vulkanObject->position.y = vulkanObject->position.y * -1.f - vulkanObject->bounds.y;
	
	if (vulkanObject->drawOrder != uiData.drawOrder) {
		vulkanObject->drawOrder = uiData.drawOrder;
		eventImmediate(EventType::UIDrawOrderUpdated, uiData.drawOrder); // notify GraphicsScene
	}
	
//	AssetDescriptor descriptor;
//	if (scene->system->assetManager->descriptor(uiData.textureAsset, descriptor) && !descriptor.uiImagePath.empty()) {
//		vulkanObject->setTexture(descriptor.uiImagePath);
//	}
	
	vulkanObject->setTexture("res/textures/ui.bmp");
}
