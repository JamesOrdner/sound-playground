#include "EObject.h"
#include "../Engine/Engine.h"
#include "../Audio/AudioEngine.h"
#include "../Audio/Components/AudioComponent.h"
#include "../UI/UIComponent.h"
#include "../Engine/EInputComponent.h"

EObject::EObject() :
	bSelected(false),
	m_scale(1)
{
}

EObject::~EObject() = default;

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
	if (m_audioComponent) m_audioComponent->transformUpdated();
}

void EObject::setVelocity(const mat::vec3& velocity)
{
	m_velocity = velocity;
	if (m_audioComponent) m_audioComponent->updateVelocity(velocity);
}

void EObject::setRotation(const mat::vec3& rotation)
{
	m_rotation = rotation;
	if (m_audioComponent) m_audioComponent->transformUpdated();
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

AudioComponent* EObject::audioComponent()
{
	return m_audioComponent.get();
}

EInputComponent* EObject::inputComponent()
{
	return m_inputComponent.get();
}

UIComponent* EObject::uiComponent()
{
	return m_uiComponent.get();
}

void EObject::addAudioComponent(std::unique_ptr<AudioComponent> component)
{
	if (m_audioComponent) removeAudioComponent();
	m_audioComponent = std::move(component);
	auto& audioEngine = Engine::instance().audio();
	audioEngine.registerComponent(m_audioComponent.get(), this);
}

void EObject::removeAudioComponent()
{
	auto& engine = Engine::instance();
	engine.audio().unregisterComponent(m_audioComponent.get());
	m_audioComponent.reset();
}
