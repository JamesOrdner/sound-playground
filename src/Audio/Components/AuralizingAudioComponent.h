#pragma once

#include "AudioComponent.h"
#include "../DSP/ADelayLine.h"
#include "../DSP/AConvolver.h"
#include <memory>

// IndirectSend represents a connection from an owning AuralizingAudioComponent to an OutputAudioComponent
// receiver. It stores all necessary, send-specific information for the indirect connection.
struct IndirectSend
{
	// AudioComponent sending indirect audio to the OutputAudioComponent
	class AuralizingAudioComponent* sender;

	// AudioComponent receiving indirect audio from the AuralizingAudioComponent
	class OutputAudioComponent* receiver;

	// Indirect impulse response from the AuralizingAudioComponent to the receiver.
	// One impulse response vector per receiver output channel.
	std::vector<float> indirectIR;

	// Convolver(s) responsible for convolving the dry AuralizingAudioComponent signal with the room IR.
	// TODO: Multichannel. One convolver per receiver output channel.
	AConvolver convolver;

	IndirectSend(class AuralizingAudioComponent* sender, class OutputAudioComponent* receiver) :
		sender(sender), receiver(receiver)
	{
	}

	void auralize() {}
};

// Forward declarations
class OutputAudioComponent;

class AuralizingAudioComponent : public AudioComponent
{
public:

	AuralizingAudioComponent();

	// Register a receiving component with this component. Called outside the audio thread.
	// Returns a pointer to the created IndirectSend object.
	IndirectSend* registerIndirectReceiver(OutputAudioComponent* receiver);

	// Unregister a receiving component from this component. Called outside the audio thread.
	void unregisterIndirectReceiver(const OutputAudioComponent* receiver);

	// AudioComponent interface
	virtual void transformUpdated() override;
	virtual void preprocess() override;

protected:

	// Process `n` output samples with auralization filters and send to indirect receivers
	void processIndirect(const float* componentOutput, size_t n);

private:

	// Sends to OutputAudioComponents receiving indirect sound from this component
	std::list<std::unique_ptr<IndirectSend>> indirectReceivers;

	// Write offset within the current buffer, reset to 0 each preprocess() call
	size_t indirectBufferOffset;
};
