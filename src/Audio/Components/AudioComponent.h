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

	// Initialize internal variables for current audio session
	virtual void init(float sampleRate, size_t channels, size_t bufferSize);

	// Clean up internals and delete any memory allocated in init()
	virtual void deinit();

	// Called when the owning EObject transform changes. Does not run in the audio thread.
	virtual void transformUpdated();

	// Called when a connected object updates its transform. Does not run in the audio thread.
	// bInput == true if this is an input to the called AudioComponent, == false if output.
	virtual void otherTransformUpdated(const ADelayLine& connection, bool bInput) {};

	// This optional function is called just before processing a full callback.
	// It can be used to prep output buffers or other internals.
	virtual void preprocess() {};

	// Pull from the input delay lines and attempt to fill output buffers and
	// and output delay lines with n samples. Returns the number of outputted
	// samples, which will often be less than n due to full output buffers or
	// running out of input samples.
	virtual size_t process(size_t n) = 0;

	// Returns the world space position of the owning object
	const mat::vec3& position() const;

	// Returns the world space velocity of the owning object
	const mat::vec3& velocity() const;

	// Returns the world space forward vector of the owning object
	const mat::vec3& forward() const;

	// Called when the velocity of the owning object changed. Does not run in the audio thread.
	void updateVelocity(const mat::vec3& velocity);

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

private:

	// Pointer to the owning EObject
	const EObject* owner;
};
