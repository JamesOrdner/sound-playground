#pragma once

#include "../SystemObjectInterface.h"
#include "../../Util/Observer.h"
#include "../../Util/Matrix.h"

class AudioObject : public SystemObjectInterface, public ObserverInterface
{
public:

	AudioObject(const class SystemSceneInterface* scene, const class UObject* uobject);

	virtual ~AudioObject();

	class AudioComponent* audioComponent;

private:

	// World space local location
	mat::vec3 position, parentPosition;

	// World space local rotation
	mat::vec3 rotation, parentRotation;

	// World space velocity
	mat::vec3 velocity, parentVelocity;
};
