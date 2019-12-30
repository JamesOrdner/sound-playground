#pragma once

#include "AudioComponent.h"

// Forward declarations
struct IndirectSend;

// OutputAudioComponent is an AudioComponent which will be used to fill
// the final output buffer, to be sent directly to the output device
class OutputAudioComponent : public AudioComponent
{
public:

	OutputAudioComponent();

	virtual ~OutputAudioComponent();

	// AudioComponent interface
	virtual void init(float sampleRate, size_t channels, size_t bufferSize) override;
	virtual void transformUpdated() override;
	virtual void preprocess() override;

	// Register a receiving component with this component. Called outside the audio thread.
	void registerIndirectSend(IndirectSend* send);

	// Unregister a receiving component from this component. Called outside the audio thread.
	void unregisterIndirectSend(IndirectSend* send);

	// Returns a pointer to the raw data of outputBuffer, which is filled during process()
	float* rawOutputBuffer();

protected:

	// Number of output channels
	size_t channels;

	// This interleaved buffer is filled during process() and will be sent directly to the output device
	std::vector<float> outputBuffer;

	// List of pointers to all AuralizingAudioComponents sends to this component
	std::list<IndirectSend*> indirectSends;
};
