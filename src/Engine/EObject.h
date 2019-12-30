#pragma once

#include "../Graphics/Matrix.h"
#include <string>
#include <memory>

// Forward declarations
class AudioComponent;

class EObject
{
public:

	EObject();

	// Name used to identify this mesh
	std::string name;

	// This is only true when the object is registered with a world
	bool bExistsInWorld;

	// Sets the world space position of the object
	virtual void setPosition(const mat::vec3& position);

	// Returns the world space position of the object
	const mat::vec3& position() const;

	// Sets the world space rotation of the object
	virtual void setRotation(const mat::vec3& rotation);

	// Returns the world space rotation of the object
	const mat::vec3& rotation() const;

	// Sets the uniform scale of the object
	virtual void setScale(float scale);

	// Sets the scale of the object
	virtual void setScale(const mat::vec3& scale);

	// Returns the object's scale
	const mat::vec3& scale() const;

	// Returns a normalized vector in the direction the object is facing
	mat::vec3 forward() const;

	// Returns the object's audio component, if present
	AudioComponent* audioComponent();

	// Returns the object's audio component as a shared pointer, if present
	std::shared_ptr<AudioComponent> audioComponentShared();

	// Registers an audio component with this object, replacing any existing component
	void addAudioComponent(const std::shared_ptr<AudioComponent>& component);

	// Removes the audio component from this object
	void removeAudioComponent();

	// Called periodically, usually at a lower rate than the frame rate
	virtual void updatePhysics(float deltaTime) {};

protected:

	// World space location
	mat::vec3 m_position;

	// World space Euler rotation
	mat::vec3 m_rotation;

	// World space scale
	mat::vec3 m_scale;

private:

	// Pointer to the object's audio component, if present
	std::shared_ptr<AudioComponent> m_audioComponent;
};
