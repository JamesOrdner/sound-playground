#include "EObject.h"
#include "Engine.h"
#include "../Audio/AudioComponent.h"

EObject::EObject() :
	bExistsInWorld(false),
	m_scale(1)
{
}

void EObject::setPosition(const mat::vec3& position)
{
	m_position = position;
}

const mat::vec3& EObject::position()
{
	return m_position;
}

void EObject::setRotation(const mat::vec3& rotation)
{
	m_rotation = rotation;
}

const mat::vec3& EObject::rotation()
{
	return m_rotation;
}

void EObject::setScale(float scale)
{
	this->m_scale = mat::vec3(scale);
}

void EObject::setScale(const mat::vec3& scale)
{
	m_scale = scale;
}

const mat::vec3& EObject::scale()
{
	return m_scale;
}

mat::vec3 EObject::forward()
{
	return mat::rotate(mat::vec3{ 0.f, 0.f, 1.f }, m_rotation);
}

std::shared_ptr<AudioComponent> EObject::audioComponent()
{
	return m_audioComponent;
}

void EObject::addAudioComponent(
	const std::shared_ptr<AudioComponent>& component,
	const std::shared_ptr<EObject>& owner)
{
	m_audioComponent = component;
	if (bExistsInWorld) {
		// Component should only be registered with audio engine if the object exists in a world
		auto& engine = Engine::instance();
		engine.audio().registerComponent(component, owner);
	}
}

void EObject::removeAudioComponent()
{
	auto& engine = Engine::instance();
	engine.audio().unregisterComponent(m_audioComponent);
	m_audioComponent.reset();
}
