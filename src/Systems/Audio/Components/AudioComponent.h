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

	// Initialize any per-delay data for this component.
	virtual void initDelayLineData(class ADelayLine* delayline, float sampleRate, bool bIsSource) {};

	// Clean up any per-delay data for this component
	virtual void deinitDelayLineData(class ADelayLine* delayline, bool bIsSource) {};

	// Called when a connected object updates its transform. Called in the audio thread.
	// bInput == true if this is an input to the called AudioComponent, == false if output.
	virtual void otherTransformUpdated(const class ADelayLine& connection, bool bInput) {};

	// Attempt to fill the provided output with at least `n` samples. Returns the number
	// of samples outputted to the buffer, which may be more or less than `n` due to
	// input buffer shortages or doppler interpolation.
	virtual size_t process(class ADelayLine* output, size_t n) { return n; };

	// Should this component accept input from other components?
	bool bAcceptsInput;

	// Should this component output to other components?
	bool bAcceptsOutput;

	// Inputs from other AudioComponents
	std::list<std::shared_ptr<class ADelayLine>> inputs;

	// Outputs to other AudioComponents
	std::list<std::shared_ptr<class ADelayLine>> outputs;

	// World space position
	mat::vec3 position;

	// World space rotation
	mat::vec3 rotation;

	// World space velocity
	mat::vec3 velocity;

	// Returns the world space forward vector of the owning object
	mat::vec3 forward() const;

	// Called externally after setting position
	virtual void transformUpdated();

protected:

	// Sample rate of the current session
	float sampleRate;

	// True after init() has been called, false after deinit()
	bool bInitialized;
};
