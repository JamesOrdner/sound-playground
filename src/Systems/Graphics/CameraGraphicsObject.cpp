#include "CameraGraphicsObject.h"
#include "../../Engine/UObject.h"

CameraGraphicsObject::CameraGraphicsObject(const UObject* uobject) :
	GraphicsObject(uobject)
{
	registerCallback(
		uobject,
		EventType::PositionUpdated,
		[this](const EventData& data) {
			position = std::get<mat::vec3>(data);
		}
	);

	registerCallback(
		uobject,
		EventType::RotationUpdated,
		[this](const EventData& data) {
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
