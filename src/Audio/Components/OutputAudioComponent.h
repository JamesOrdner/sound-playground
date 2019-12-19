#pragma once

#include "AudioComponent.h"

// OutputAudioComponent is an AudioComponent which will be used to fill
// the final output buffer, to be sent directly to the output device
class OutputAudioComponent : public AudioComponent
{
public:

	OutputAudioComponent();

	// This interleaved buffer is filled during process() and will be sent directly to the output device
	std::vector<float> outputBuffer;

	// Returns the outputBuffer, which is filled during process().
	// This must be called only after a complete call to process().
	const std::vector<float>& collectOutput();

	// AudioComponent interface
	virtual void init(float sampleRate, size_t channels, size_t bufferSize) override;

protected:

	// Number of output channels
	size_t channels;
};
