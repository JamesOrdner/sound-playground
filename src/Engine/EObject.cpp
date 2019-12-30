#include "EObject.h"
#include "Engine.h"
#include "../Audio/Components/AudioComponent.h"

EObject::EObject() :
	bExistsInWorld(false),
	m_scale(1)
{
}

void EObject::setPosition(const mat::vec3& position)
{
	m_position = position;
	if (m_audioComponent) m_audioComponent->transformUpdated();
}

const mat::vec3& EObject::position() const
{
	return m_position;
}

void EObject::setRotation(const mat::vec3& rotation)
{
	m_rotation = rotation;
	if (m_audioComponent) m_audioComponent->transformUpdated();
}

const mat::vec3& EObject::rotation() const
{
	return m_rotation;
}

void EObject::setScale(float scale)
{
	this->m_scale = mat::vec3(scale);
	if (m_audioComponent) m_audioComponent->transformUpdated();
}

void EObject::setScale(const mat::vec3& scale)
{
	m_scale = scale;
	if (m_audioComponent) m_audioComponent->transformUpdated();
}

const mat::vec3& EObject::scale() const
{
	return m_scale;
}

mat::vec3 EObject::forward() const
{
	return mat::rotate(mat::vec3{ 0.f, 0.f, 1.f }, m_rotation);
}

AudioComponent* EObject::audioComponent()
{
	return m_audioComponent.get();
}

std::shared_ptr<AudioComponent> EObject::audioComponentShared()
{
	return m_audioComponent;
}

void EObject::addAudioComponent(const std::shared_ptr<AudioComponent>& component)
{
	m_audioComponent = component;
	if (bExistsInWorld) {
		// Component should only be registered with audio engine if the object exists in a world
		auto& engine = Engine::instance();
		engine.audio().registerComponent(component, this);
	}
}

void EObject::removeAudioComponent()
{
	auto& engine = Engine::instance();
	engine.audio().unregisterComponent(m_audioComponent);
	m_audioComponent.reset();
}
