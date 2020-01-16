#pragma once

#include "InputObject.h"
#include "../Util/Matrix.h"

class CameraInputObject : public InputObject
{
public:

	CameraInputObject(const class UObject* uobject);

	void handleEvent(const SDL_Event& sdlEvent) override;

	void tick(float deltaTime) override;

private:

	mat::vec3 pivotPosition;

	mat::vec3 pivotRotation;

	// Scalar distance of camera from object pivot position
	float pivotDistance;
	float maxPivotDistance;

	// True when the camera is responding to mouse motion events
	bool bOrbiting;

	// Velocity of the camera relative to the camera rotation, left hand coordinate system (+z forward)
	mat::vec3 cameraVelocity;

	// Camera movement multiplier
	float movementSpeedMultiplier;

	void broadcastPositionChange();
	void broadcastRotationChange();
};
