#include "UIGraphicsObject.h"
#include "GraphicsScene.h"
#include "../../Engine/UObject.h"

UIGraphicsObject::UIGraphicsObject(const SystemSceneInterface* scene, const UObject* uobject) :
	GraphicsObject(scene, uobject)
{
	registerCallback(
		uobject,
		EventType::UIPositionUpdated,
		[this](const EventData& data, bool bEventFromParent) {
			uiData.position = std::get<mat::vec2>(data);
		}
	);

	registerCallback(
		uobject,
		EventType::UIBoundsUpdated,
		[this](const EventData& data, bool bEventFromParent) {
			uiData.bounds = std::get<mat::vec2>(data);
		}
	);

	registerCallback(
		uobject,
		EventType::UIDrawOrderUpdated,
		[this](const EventData& data, bool bEventFromParent) {
			uiData.drawOrder = std::get<uint32_t>(data);
			eventImmediate(EventType::UIDrawOrderUpdated, data); // notify GraphicsScene
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
		}
	);

	registerCallback(
		uobject,
		EventType::UITextureBoundsUpdated,
		[this](const EventData& data, bool bEventFromParent) {
			uiData.textureBounds = std::get<mat::vec2>(data);
		}
	);
}
