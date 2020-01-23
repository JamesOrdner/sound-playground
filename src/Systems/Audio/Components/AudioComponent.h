#pragma once

#include "../../../Util/Matrix.h"
#include <vector>
#include <list>
#include <memory>

class AudioComponent
{
public:

	AudioComponent();

	virtual ~AudioComponent();

	// Initialize internal variables for current audio session
	virtual void init(float sampleRate);

	// Clean up internals and delete any memory allocated in init()
	virtual void deinit();

	// Called when a connected object updates its transform. Called in the audio thread.
	// bInput == true if this is an input to the called AudioComponent, == false if output.
	virtual void otherTransformUpdated(const class ADelayLine& connection, bool bInput) {};

	// Pull from the input delay lines and attempt to fill output buffers and
	// and output delay lines with n samples. Returns the number of outputted
	// samples, which will often be less than n due to full output buffers or
	// running out of input samples.
	virtual size_t process(size_t n) { return n; };

	// Should this component accept input from other components?
	bool bAcceptsInput;

	// Should this component output to other components?
	bool bAcceptsOutput;

	// Inputs from other AudioComponents
	std::list<std::shared_ptr<ADelayLine>> inputs;

	// Outputs to other AudioComponents
	std::list<std::shared_ptr<ADelayLine>> outputs;

	// World space position
	mat::vec3 position;

	// World space rotation
	mat::vec3 rotation;

	// World space velocity
	mat::vec3 velocity;

	// Returns the world space forward vector of the owning object
	mat::vec3 forward() const;

protected:

	// Sample rate of the current session
	float sampleRate;

	// True after init() has been called, false after deinit()
	bool bInitialized;

	virtual void transformUpdated();
	virtual void velocityUpdated();
};
