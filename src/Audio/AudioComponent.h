#pragma once

#include <vector>
#include <list>
#include <memory>

// Forward declarations
class EObject;

namespace mat
{
	struct vec3;
}

class AudioComponent
{
public:
	
	AudioComponent(size_t bufferSize);

	// Returns the world space position of the owning object
	const mat::vec3& getPosition();

	// Fill all output delay lines and the irInputBuffer with n samples
	void processOutputs(size_t n);

	// Process room IR filter and add stereo result to interleaved buffer
	void processIndirect(std::vector<float>& buffer, size_t n);

protected:

	// This mono buffer is filled during processOutputs and fed to the room IR filter
	std::vector<float> omniOutputBuffer;

private:

	// Weak pointer to the owning EObject
	std::weak_ptr<EObject> owner;
};

