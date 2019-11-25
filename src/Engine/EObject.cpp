#include "Engine.h"
#include "EObject.h"

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

void EObject::addAudioComponent(const std::shared_ptr<AudioComponent>& component)
{
	_audioComponent = component;
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
