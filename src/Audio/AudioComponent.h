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
public:

	// Initialize internal variables for current audio session
	virtual void init(size_t bufferSize, size_t channels);

	// Returns the world space position of the owning object
	const mat::vec3& getPosition();

	// Pull from the input delay lines and fill the omniOutputBuffer
	// and all output delay lines with n samples
	virtual void process(size_t n) = 0;

protected:

	// Inputs from other AudioComponents
	std::list<std::shared_ptr<ADelayLine>> inputs;

	// Outputs to other AudioComponents
	std::list<std::shared_ptr<ADelayLine>> outputs;

	// This mono buffer is filled during process() and fed to the room IR filter
	std::vector<float> indirectInputBuffer;

private:

	// Weak pointer to the owning EObject
	std::weak_ptr<EObject> owner;
};
