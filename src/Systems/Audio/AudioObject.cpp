#include "AudioObject.h"
#include "../../Engine/UObject.h"
#include "Components/AudioComponent.h"

AudioObject::AudioObject(const SystemSceneInterface* scene, const UObject* uobject) :
	SystemObjectInterface(scene, uobject)
{
	registerCallback(
		uobject,
		EventType::PositionUpdated,
		[this](const EventData& data, bool bEventFromParent) {
			(bEventFromParent ? parentPosition : position) = std::get<mat::vec3>(data);
			this->uobject->childEventImmediate(EventType::PositionUpdated, position + parentPosition);
		}
	);

	registerCallback(
		uobject,
		EventType::RotationUpdated,
		[this](const EventData& data, bool bEventFromParent) {
			(bEventFromParent ? parentRotation : rotation) = std::get<mat::vec3>(data);
			this->uobject->childEventImmediate(EventType::RotationUpdated, rotation + parentRotation);
		}
	);
}

AudioObject::~AudioObject()
{
}
