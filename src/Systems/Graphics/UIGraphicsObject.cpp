#include "UIGraphicsObject.h"
#include "GraphicsScene.h"
#include "../../Engine/UObject.h"
#include "Vulkan/VulkanUI.h"

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

// TODO: TEMP
constexpr mat::vec2 resolution{ 1280.f, 720.f };

void UIGraphicsObject::dataUpdated()
{
	if (!vulkanObject) return;
	
	// TODO: TEMP
	if (uiData.bounds.y > 700) {
		vulkanObject->bounds = mat::vec2(0.f);
		return;
	}
	
	vulkanObject->position = uiData.position / resolution * 2.f - 1.f;
	vulkanObject->bounds = uiData.bounds / resolution * 2.f;
	
	vulkanObject->uv_position = uiData.texturePosition / resolution;
	vulkanObject->uv_bounds = uiData.textureBounds / resolution;
	
	// flip y axis
	vulkanObject->position.y = vulkanObject->position.y * -1.f - vulkanObject->bounds.y;
	
	if (vulkanObject->drawOrder != uiData.drawOrder) {
		vulkanObject->drawOrder = uiData.drawOrder;
		eventImmediate(EventType::UIDrawOrderUpdated, uiData.drawOrder); // notify GraphicsScene
	}
}
