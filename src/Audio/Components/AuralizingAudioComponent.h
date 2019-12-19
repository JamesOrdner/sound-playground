#pragma once

#include "AudioComponent.h"
#include "../DSP/ADelayLine.h"
#include "../DSP/AConvolver.h"

// Forward declarations
class OutputAudioComponent;

// AuralSend represents a connection from an owning AuralizingAudioComponent to an OutputAudioComponent
// receiver. It stores all necessary, send-specific information for the indirect connection.
struct AuralSend
{
	// AudioComponent receiving indirect audio from the AuralizingAudioComponent
	OutputAudioComponent* receiver;

	// Indirect impulse response from the AuralizingAudioComponent to the receiver.
	// One impulse response vector per receiver output channel.
	std::vector<std::vector<float>> indirectIR;

	// Convolver(s) responsible for convolving the dry AuralizingAudioComponent signal with the room IR.
	// One convolver per receiver output channel.
	std::vector<AConvolver> convolver;
};

class AuralizingAudioComponent : public AudioComponent
{
public:

	// Register a receiving component with this component. Called outside the audio thread.
	void registerReceiver(OutputAudioComponent* receiver);

	// Unregister a receiving component from this component. Called outside the audio thread.
	void unregisterReceiver(const OutputAudioComponent* receiver);

	// Iterate all OutputAudioComponent indirect sends and contribute to their output. This must be called after the
	// usual process() routine has completed and indirectBuffer filled with a full buffer size's worth of samples.
	void processIndirect();

	// AudioComponent interface
	virtual void init(float sampleRate, size_t channels, size_t bufferSize) override;

protected:

	// This mono buffer is filled during process() and fed to the room IR filter
	ReadWriteBuffer indirectBuffer;

private:

	// AudioComponents receiving indirect sound from this component
	std::list<AuralSend> receivers;

	// Buffer needed for efficient processing
	std::vector<float> processIndirectBuffer;
};
