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

public:

	AudioComponent();

	// Initialize internal variables for current audio session
	virtual void init(size_t bufferSize, size_t channels);

	// Clean up internals and delete any memory allocated in init()
	virtual void deinit() {};
	
	// Size in samples of the shortest input buffer
	size_t shortestInput();

	// Returns the world space position of the owning object
	const mat::vec3& position();

	// Returns the world space forward vector of the owning object
	const mat::vec3& forward();

	// Called when the owning EObject transform changes
	void transformUpdated();

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
	std::weak_ptr<EObject> m_owner;

	// World space position of this object, updated by AudioEngine. It is not
	// guaranteed to always be in sync with the actual owning object's location.
	mat::vec3 m_position;

	// World space forward vector of this object, updated by AudioEngine. It is not
	// guaranteed to always be in sync with the actual owning object's forward vector.
	mat::vec3 m_forward;

	// Marked true when the owning EObject's transform has been modified
	bool bDirtyTransform;
};
