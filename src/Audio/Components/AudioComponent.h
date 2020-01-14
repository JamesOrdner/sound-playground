#pragma once

#include "../../Graphics/Matrix.h"
#include <vector>
#include <list>
#include <memory>

// Forward declarations
class EObject;
class ADelayLine;

class AudioComponent
{
	friend class AudioEngine;

public:

	AudioComponent();

	virtual ~AudioComponent();

	// TEMP for StateManager testing
	void setOwner(const EObject* owner);

	// Initialize internal variables for current audio session
	virtual void init(float sampleRate);

	// Clean up internals and delete any memory allocated in init()
	virtual void deinit();

	// Called when a connected object updates its transform. Called in the audio thread.
	// bInput == true if this is an input to the called AudioComponent, == false if output.
	virtual void otherTransformUpdated(const ADelayLine& connection, bool bInput) {};

	// Pull from the input delay lines and attempt to fill output buffers and
	// and output delay lines with n samples. Returns the number of outputted
	// samples, which will often be less than n due to full output buffers or
	// running out of input samples.
	virtual size_t process(size_t n) { return n; };

	// Returns the world space position of the component
	const mat::vec3& componentPosition() const;

	// Returns the world space velocity of the owning object
	const mat::vec3& componentVelocity() const;

	// Returns the world space forward vector of the owning object
	mat::vec3 forward() const;

protected:

	// Should this component accept input from other components?
	bool bAcceptsInput;

	// Should this component output to other components?
	bool bAcceptsOutput;

	// Sample rate of the current session
	float sampleRate;

	// True after init() has been called, false after deinit()
	bool bInitialized;

	// Inputs from other AudioComponents
	std::list<std::shared_ptr<ADelayLine>> inputs;

	// Outputs to other AudioComponents
	std::list<std::shared_ptr<ADelayLine>> outputs;

	virtual void transformUpdated();
	virtual void velocityUpdated();

private:
	
	mat::vec3 position;
	mat::vec3 velocity;

	unsigned int ids[2]; // TEMP
};
