#pragma once

#include "../Graphics/Matrix.h"
#include <vector>
#include <list>
#include <memory>

// Forward declarations
class EObject;
struct ADelayLine;

class AudioComponent
{
	friend class AudioEngine;
	friend class EObject;

public:

	// Initialize internal variables for current audio session
	virtual void init(size_t bufferSize, size_t channels);

	// Clean up internals and delete any memory allocated in init()
	virtual void deinit() {};
	
	// Size in samples of the shortest input buffer
	size_t shortestInput();

	// Returns the world space position of the owning object
	const mat::vec3& getPosition();

	// This optional function is called just before processing a full callback.
	// It can be used to prep output buffers or other internals.
	virtual void preprocess() {};

	// Pull from the input delay lines and attempt to fill the omniOutputBuffer
	// and all output delay lines with n samples. Returns the number of outputted
	// samples, which will often be less than n due to full output buffers or
	// running out of input samples.
	virtual size_t process(size_t n) = 0;

protected:

	// Should this component accept input from other components?
	bool bAcceptsInput;

	// Should this component output to other components?
	bool bAcceptsOutput;

	// Inputs from other AudioComponents
	std::list<std::shared_ptr<ADelayLine>> inputs;

	// Returns the least number of currently available samples across all inputs
	size_t pullCount();

	// Outputs to other AudioComponents
	std::list<std::shared_ptr<ADelayLine>> outputs;

	// Returns the least free buffer space across all outputs
	size_t pushCount();

	// This mono buffer is filled during process() and fed to the room IR filter
	std::vector<float> indirectInputBuffer;

private:

	// Weak pointer to the owning EObject
	std::weak_ptr<EObject> owner;
};
