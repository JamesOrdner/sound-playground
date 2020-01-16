#include "CameraGraphicsObject.h"
#include "../Managers/StateManager.h"

CameraGraphicsObject::CameraGraphicsObject(const UObject* uobject) :
	GraphicsObject(uobject)
{
	auto& stateManager = StateManager::instance();

	observerIDs.push_back(
		stateManager.registerObserver(
			uobject,
			StateManager::EventType::PositionUpdated,
			[this](const StateManager::EventData& data) {
				position = std::get<mat::vec3>(data);
			}
		)
	);

	observerIDs.push_back(
		stateManager.registerObserver(
			uobject,
			StateManager::EventType::RotationUpdated,
			[this](const StateManager::EventData& data) {
				rotation = std::get<mat::vec3>(data);
			}
		)
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
