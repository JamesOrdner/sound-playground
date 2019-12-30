#pragma once

#include "AudioComponent.h"
#include "GeneratingAudioComponent.h"
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
	std::vector<std::vector<float>> indirectIRs;

	// Convolver(s) responsible for convolving the dry AuralizingAudioComponent signal with the room IR.
	// One convolver per receiver output channel.
	std::vector<AConvolver> convolvers;

	IndirectSend() : sender(nullptr), receiver(nullptr) {}

	void auralize() {}
};

// Forward declarations
class OutputAudioComponent;

class AuralizingAudioComponent : public AudioComponent, public GeneratingAudioComponent
{
public:

	AuralizingAudioComponent();

	virtual ~AuralizingAudioComponent();

	// AudioComponent interface
	virtual void transformUpdated() override;
	virtual void init(float sampleRate, size_t channels, size_t bufferSize) override;
	virtual void deinit() override;
	virtual void preprocess() override;

	// Process `n` output samples with auralization filters and send to indirect receivers
	void processIndirect(size_t n);

	// Return the number of indirect frames processed since the last preprocess() call
	size_t indirectFramesProcessed();

	// Register a receiving component with this component. Called outside the audio thread.
	// Returns a pointer to the created IndirectSend object.
	IndirectSend* registerIndirectReceiver(OutputAudioComponent* receiver);

	// Unregister a receiving component from this component. Called outside the audio thread.
	void unregisterIndirectReceiver(const OutputAudioComponent* receiver);

private:

	// Sends to OutputAudioComponents receiving indirect sound from this component
	std::list<std::unique_ptr<IndirectSend>> indirectReceivers;

	// This buffer is required for processIndirect()
	std::vector<float> processIndirectBuffer;

	// Write offset within the current buffer, reset to 0 each preprocess() call
	size_t indirectBufferOffset;

	// Unique ID which associates the AuralizingAudioComponent with the GeneratingAudioComponent
	unsigned int genID;
};
