#include "ECamera.h"
#include "../Engine/EInputComponent.h"

ECamera::ECamera() :
	prevCursorX(0),
	prevCursorY(0),
	movementScale(0.08f),
	bOrbiting(false),
	pivotDistance(5.f),
	maxPivotDistance(10.f)
{
	setupInput();
}

void ECamera::setupInput()
{
	m_inputComponent = std::make_unique<EInputComponent>();

	m_inputComponent->registerKeyCallback(
		EInputKeyEvent{ SDLK_w, SDL_KEYDOWN },
		[this] { cameraVelocity.z = 1.f;  }
	);

	m_inputComponent->registerKeyCallback(
		EInputKeyEvent{ SDLK_w, SDL_KEYUP },
		[this] { cameraVelocity.z = 0.f;  }
	);

	m_inputComponent->registerKeyCallback(
		EInputKeyEvent{ SDLK_s, SDL_KEYDOWN },
		[this] { cameraVelocity.z = -1.f;  }
	);

	m_inputComponent->registerKeyCallback(
		EInputKeyEvent{ SDLK_s, SDL_KEYUP },
		[this] { cameraVelocity.z = 0.f;  }
	);

	m_inputComponent->registerKeyCallback(
		EInputKeyEvent{ SDLK_d, SDL_KEYDOWN },
		[this] { cameraVelocity.x = 1.f;  }
	);

	m_inputComponent->registerKeyCallback(
		EInputKeyEvent{ SDLK_d, SDL_KEYUP },
		[this] { cameraVelocity.x = 0.f;  }
	);

	m_inputComponent->registerKeyCallback(
		EInputKeyEvent{ SDLK_a, SDL_KEYDOWN },
		[this] { cameraVelocity.x = -1.f;  }
	);

	m_inputComponent->registerKeyCallback(
		EInputKeyEvent{ SDLK_a, SDL_KEYUP },
		[this] { cameraVelocity.x = 0.f;  }
	);

	m_inputComponent->registerMouseButtonCallback(
		EInputMouseButtonEvent{ SDL_BUTTON_MIDDLE, SDL_MOUSEBUTTONDOWN },
		[this] { bOrbiting = true; }
	);

	m_inputComponent->registerMouseButtonCallback(
		EInputMouseButtonEvent{ SDL_BUTTON_MIDDLE, SDL_MOUSEBUTTONUP },
		[this] { bOrbiting = false; }
	);

	m_inputComponent->registerScrollUpCallback(
		[this]() { changePivotDist(-1.f); }
	);

	m_inputComponent->registerScrollDownCallback(
		[this]() { changePivotDist(1.f); }
	);

	m_inputComponent->registerCursorCallback(
		[this](int x, int y) { orbit(x, y); }
	);
}

void ECamera::changePivotDist(float deltaDist)
{
	pivotDistance += deltaDist * (pivotDistance / maxPivotDistance);
	pivotDistance = std::fminf(std::fmaxf(pivotDistance, 0.3f), maxPivotDistance);
}

void ECamera::orbit(int x, int y)
{
	int deltaX = x - prevCursorX;
	int deltaY = y - prevCursorY;
	prevCursorX = x;
	prevCursorY = y;

	if (!bOrbiting) return;
	float xRot = m_rotation.x + static_cast<float>(deltaY) * 0.003f;
	m_rotation.x = std::fminf(std::fmaxf(xRot, mat::pi * 0.02f), mat::pi * 0.45f);
	m_rotation.y -= static_cast<float>(deltaX) * 0.003f;
}

const mat::vec3& ECamera::cameraFocus() const
{
	return m_position;
}

mat::vec3 ECamera::cameraPosition() const
{
	return m_position - forward() * pivotDistance;
}

void ECamera::tick(float deltaTime)
{
	EObject::tick(deltaTime);
	
	// Forward/back motion
	mat::vec3 fDir = forward();
	fDir.y = 0;
	fDir = mat::normal(fDir);
	mat::vec3 rDir = mat::cross(fDir, mat::vec3{ 0, 1, 0 });
	float scale = movementScale * ((pivotDistance / maxPivotDistance) * 0.9f + 0.1f);
	m_position = m_position + (fDir * cameraVelocity.z + rDir * cameraVelocity.x) * scale;
}
