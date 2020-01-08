#pragma once

#include "EObject.h"

class ECamera : public EObject
{
public:

	ECamera();

	// Returns the position which the camera is looking at
	const mat::vec3& cameraFocus() const;

	// Returns the position of the camera
	mat::vec3 cameraPosition() const;

	// EObject interface
	void tick(float deltaTime) override;

private:

	void setupInput();
	void changePivotDist(float deltaDist);
	void orbit(int x, int y);

	// Stores the last cursor position, to calculate delta position
	int prevCursorX, prevCursorY;

	// Camera movement multiplier
	float movementScale;

	// True when the input to orbit the camera is held
	bool bOrbiting;

	// Scalar distance of camera from object position
	float pivotDistance;

	float maxPivotDistance;

	// Velocity of the camera relative to the camera rotation, left hand coordinate system (+z forward)
	mat::vec3 cameraVelocity;
};
