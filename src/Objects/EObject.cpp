#include "EObject.h"
#include "../Engine/Engine.h"
#include "../Audio/AudioEngine.h"
#include "../Audio/Components/AudioComponent.h"
#include "../UI/UIComponent.h"
#include "../Engine/EInputComponent.h"
#include "../Managers/StateManager.h"

EObject::EObject() :
	bSelected(false),
	m_scale(1),
	audioComponent(nullptr)
{
}

EObject::~EObject()
{
	if (audioComponent) {
		StateManager::instance().event(
			audioComponent,
			StateManager::EventType::ComponentDeleted,
			audioComponent);
	}
}

void EObject::setSelected(bool selected)
{
	bSelected = selected;
}

bool EObject::selected() const
{
	return bSelected;
}

void EObject::setPosition(const mat::vec3& position)
{
	m_position = position;
	StateManager::instance().event(this, StateManager::EventType::PositionUpdated, position);
}

void EObject::setVelocity(const mat::vec3& velocity)
{
	m_velocity = velocity;
	StateManager::instance().event(this, StateManager::EventType::VelocityUpdated, velocity);
}

void EObject::setRotation(const mat::vec3& rotation)
{
	m_rotation = rotation;
	StateManager::instance().event(this, StateManager::EventType::RotationUpdated, rotation);
}

void EObject::setScale(float scale)
{
	m_scale = mat::vec3(scale);
	StateManager::instance().event(this, StateManager::EventType::ScaleUpdated, m_scale);
}

void EObject::setScale(const mat::vec3& scale)
{
	m_scale = scale;
	StateManager::instance().event(this, StateManager::EventType::ScaleUpdated, scale);
}

const mat::vec3& EObject::position() const
{
	return m_position;
}

const mat::vec3& EObject::velocity() const
{
	return m_velocity;
}

const mat::vec3& EObject::rotation() const
{
	return m_rotation;
}

const mat::vec3& EObject::scale() const
{
	return m_scale;
}

mat::vec3 EObject::forward() const
{
	return mat::rotate(mat::vec3{ 0.f, 0.f, 1.f }, m_rotation);
}

EInputComponent* EObject::inputComponent()
{
	return m_inputComponent.get();
}

UIComponent* EObject::uiComponent()
{
	return m_uiComponent.get();
}
