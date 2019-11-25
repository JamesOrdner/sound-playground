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
	virtual void setPosition(const mat::vec3& location);

	// Returns the world space position of the object
	const mat::vec3& getPosition();

	// Sets the uniform scale of the object
	virtual void setScale(float scale);

	// Sets the scale of the object
	virtual void setScale(const mat::vec3& scale);

	// Returns the object's scale
	const mat::vec3& getScale();

	// Returns the object's audio component, if present
	std::shared_ptr<AudioComponent> audioComponent();

	// Registers an audio component with this object, replacing any existing component
	void addAudioComponent(const std::shared_ptr<AudioComponent>& component);

	// Removes the audio component from this object
	void removeAudioComponent();

protected:

	// World space location
	mat::vec3 position;

	// World space scale
	mat::vec3 scale;

private:

	// Pointer to the object's audio component, if present
	std::shared_ptr<AudioComponent> _audioComponent;
};

