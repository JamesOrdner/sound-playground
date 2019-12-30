#pragma once

#include "AuralizingAudioComponent.h"

#include <memory>

// Forward declarations
class AConvolver;

class ASpeaker : public AuralizingAudioComponent
{
public:

	ASpeaker();

	// AudioComponent interface
	void init(float sampleRate, size_t channels, size_t bufferSize) override;
	void deinit() override;
	size_t process(size_t n) override;

private:

	// Convolves the speaker signal with the speaker IR
	std::unique_ptr<AConvolver> convolver;

	// Buffer required for process()
	std::vector<float> workingBuffer;

	// GeneratingAudioComponent interface
	size_t generateImpl(float* buffer, size_t count) override;
};
