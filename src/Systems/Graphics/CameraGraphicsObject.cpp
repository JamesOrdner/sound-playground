#include "CameraGraphicsObject.h"
#include "../../Engine/UObject.h"

CameraGraphicsObject::CameraGraphicsObject(const SystemSceneInterface* scene, const UObject* uobject) :
	GraphicsObject(scene, uobject)
{
	registerCallback(
		uobject,
		EventType::PositionUpdated,
		[this](const EventData& data, bool bEventFromParent) {
			position = std::get<mat::vec3>(data);
		}
	);

	registerCallback(
		uobject,
		EventType::RotationUpdated,
		[this](const EventData& data, bool bEventFromParent) {
			rotation = std::get<mat::vec3>(data);
		}
	);
}

const mat::vec3& CameraGraphicsObject::cameraPosition() const
{
	return position;
}

mat::vec3 CameraGraphicsObject::cameraForward() const
{
	return mat::forward(rotation);
}
