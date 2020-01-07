#pragma once

#include "../Graphics/Matrix.h"
#include <string>
#include <memory>

// Forward declarations
class EInputComponent;
class UIComponent;
class AudioComponent;

class EObject
{
public:

	EObject();

	virtual ~EObject();

	// Name used to identify this object
	std::string name;

	// This is only true when the object is registered with a world
	bool bExistsInWorld;

	virtual void setSelected(bool selected);
	bool selected() const;

	virtual void setPosition(const mat::vec3& position);
	virtual void setVelocity(const mat::vec3& velocity);
	virtual void setRotation(const mat::vec3& rotation);
	virtual void setScale(float scale);
	virtual void setScale(const mat::vec3& scale);
	const mat::vec3& position() const;
	const mat::vec3& velocity() const;
	const mat::vec3& rotation() const;
	const mat::vec3& scale() const;

	// Returns a normalized vector in the direction the object is facing
	mat::vec3 forward() const;

	// Returns the object's audio component, if present
	AudioComponent* audioComponent();

	// Returns the object's input component, if present
	EInputComponent* inputComponent();

	// Returns the object's UI component, if present
	UIComponent* uiComponent();

	// Registers an audio component with this object, replacing any existing component
	void addAudioComponent(std::unique_ptr<AudioComponent> component);

	// Removes the audio component from this object
	void removeAudioComponent();

	// Called once per frame
	virtual void tick(float deltaTime) {};

	// Called periodically, usually at a lower rate than the frame rate
	virtual void updatePhysics(float deltaTime) {};

protected:

	// Is this object currently selected?
	bool bSelected;

	// World space location
	mat::vec3 m_position;

	// World space velocity
	mat::vec3 m_velocity;

	// World space Euler rotation
	mat::vec3 m_rotation;

	// World space scale
	mat::vec3 m_scale;

	// Object's input component (optional)
	std::unique_ptr<EInputComponent> m_inputComponent;

	// Object's UI component (optional)
	std::unique_ptr<UIComponent> m_uiComponent;

private:

	// Pointer to the object's audio component, if present
	std::unique_ptr<AudioComponent> m_audioComponent;
};
