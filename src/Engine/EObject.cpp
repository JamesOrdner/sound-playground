#include "EObject.h"
#include "Engine.h"
#include "../Audio/AudioComponent.h"

EObject::EObject() :
	bExistsInWorld(false),
	scale(1)
{
}

void EObject::setPosition(const mat::vec3& location)
{
	this->position = location;
}

const mat::vec3& EObject::getPosition()
{
	return position;
}

void EObject::setRotation(const mat::vec3& rotation)
{
	this->rotation = rotation;
}

const mat::vec3& EObject::getRotation()
{
	return rotation;
}

void EObject::setScale(float scale)
{
	this->scale = mat::vec3(scale);
}

void EObject::setScale(const mat::vec3& scale)
{
	this->scale = scale;
}

const mat::vec3& EObject::getScale()
{
	return scale;
}

std::shared_ptr<AudioComponent> EObject::audioComponent()
{
	return _audioComponent;
}

void EObject::addAudioComponent(
	const std::shared_ptr<AudioComponent>& component,
	const std::shared_ptr<EObject>& owner)
{
	_audioComponent = component;
	_audioComponent->owner = owner;
	if (bExistsInWorld) {
		// Component should only be registered with audio engine if the object exists in a world
		auto& engine = Engine::instance();
		engine.audio().registerComponent(component);
	}
}

void EObject::removeAudioComponent()
{
	auto& engine = Engine::instance();
	engine.audio().unregisterComponent(_audioComponent);
	_audioComponent.reset();
}
