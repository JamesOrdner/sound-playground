#include "CameraInputObject.h"
#include "../../Engine/UObject.h"

CameraInputObject::CameraInputObject(const UObject* uobject) :
	InputObject(uobject),
	pivotDistance(5.f),
	maxPivotDistance(10.f),
	bOrbiting(false),
	movementSpeedMultiplier(2.f)
{
	// Update camera with initial configuration
	broadcastPositionChange();
	broadcastRotationChange();
}

void CameraInputObject::handleEvent(const SDL_Event& sdlEvent)
{
	InputObject::handleEvent(sdlEvent);

	if (sdlEvent.type == SDL_KEYDOWN) {
		switch (sdlEvent.key.keysym.sym) {
		case SDLK_w: cameraVelocity.z =  1.f; break;
		case SDLK_s: cameraVelocity.z = -1.f; break;
		case SDLK_d: cameraVelocity.x =  1.f; break;
		case SDLK_a: cameraVelocity.x = -1.f; break;
		}
	}
	else if (sdlEvent.type == SDL_KEYUP) {
		switch (sdlEvent.key.keysym.sym) {
		case SDLK_w: cameraVelocity.z = 0.f; break;
		case SDLK_s: cameraVelocity.z = 0.f; break;
		case SDLK_d: cameraVelocity.x = 0.f; break;
		case SDLK_a: cameraVelocity.x = 0.f; break;
		}
	}
	else if (sdlEvent.type == SDL_MOUSEMOTION) {
		if (!bOrbiting) return;
		float xRot = pivotRotation.x + static_cast<float>(sdlEvent.motion.yrel) * 0.003f;
		pivotRotation.x = std::fminf(std::fmaxf(xRot, mat::pi * 0.02f), mat::pi * 0.45f);
		pivotRotation.y -= static_cast<float>(sdlEvent.motion.xrel) * 0.003f;
		broadcastPositionChange();
		broadcastRotationChange();
	}
	else if (sdlEvent.type == SDL_MOUSEBUTTONDOWN || sdlEvent.type == SDL_MOUSEBUTTONUP) {
		if (sdlEvent.button.button == SDL_BUTTON_MIDDLE) {
			bOrbiting = sdlEvent.type == SDL_MOUSEBUTTONDOWN;
		}
	}
	else if (sdlEvent.type == SDL_MOUSEWHEEL) {
		pivotDistance -= sdlEvent.wheel.y * (pivotDistance / maxPivotDistance);
		pivotDistance = std::fminf(std::fmaxf(pivotDistance, 0.3f), maxPivotDistance);
		broadcastPositionChange();
	}
}

void CameraInputObject::tick(float deltaTime)
{
	InputObject::tick(deltaTime);

	if (cameraVelocity.x != 0.f || cameraVelocity.z != 0.f) {
		mat::vec3 fDir = mat::forward(pivotRotation);
		fDir.y = 0;
		fDir = mat::normal(fDir);
		mat::vec3 rDir = mat::cross(fDir, mat::vec3{ 0, 1, 0 });
		float scale = movementSpeedMultiplier * ((pivotDistance / maxPivotDistance) * 0.9f + 0.1f) * deltaTime;
		pivotPosition += (fDir * cameraVelocity.z + rDir * cameraVelocity.x) * scale;

		broadcastPositionChange();
	}
}

void CameraInputObject::broadcastPositionChange()
{
	uobject->event(EventType::PositionUpdated, pivotPosition - mat::forward(pivotRotation) * pivotDistance);
}

void CameraInputObject::broadcastRotationChange()
{
	uobject->event(EventType::RotationUpdated, pivotRotation);
}
