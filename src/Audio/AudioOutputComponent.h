#pragma once

#include "AudioComponent.h"

// AudioOutputComponent is an AudioComponent which will be used to fill
// the final output buffer, normally with no outputs to other components
class AudioOutputComponent : public AudioComponent
{
public:

	virtual void init(size_t bufferSize, size_t channels) override;

	// Returns the outputBuffer, which is filled during process().
	// This must be called only after a complete call to process().
	const std::vector<float>& collectOutput();

protected:

	// Number of output channels
	size_t channels;

	// This interleaved buffer is filled during process() and will be sent directly to the output device
	std::vector<float> outputBuffer;
};
