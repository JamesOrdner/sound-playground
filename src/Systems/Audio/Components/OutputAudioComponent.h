#pragma once

#include "AudioComponent.h"

// OutputAudioComponent is an AudioComponent which will be used to fill
// the final output buffer, to be sent directly to the output device
class OutputAudioComponent : public AudioComponent
{
public:

	OutputAudioComponent();

	virtual ~OutputAudioComponent();

	// Register a receiving component with this component. Called outside the audio thread.
	void registerIndirectSend(struct IndirectSend* send);

	// Unregister a receiving component from this component. Called outside the audio thread.
	void unregisterIndirectSend(struct IndirectSend* send);

	// Contribute `n` frames to interleaved buffer `buffer`. Samples should be constructively
	// added to `buffer`, rather than overridden, as `buffer` is shared by all output components.
	virtual size_t processOutput(float* buffer, size_t n) = 0;

	// AudioComponent interface
	virtual void transformUpdated() override;

protected:

	// List of pointers to all AuralizingAudioComponents sends to this component
	std::list<IndirectSend*> indirectSends;
};
