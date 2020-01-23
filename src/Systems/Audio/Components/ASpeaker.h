#pragma once

#include "AuralizingAudioComponent.h"
#include <memory>

class ASpeaker : public AudioComponent // AuralizingAudioComponent
{
public:

	ASpeaker();

	// AudioComponent interface
	// void init(float sampleRate) override;
	// void deinit() override;
	size_t process(size_t n) override;

private:

	// Convolves the speaker signal with the speaker IR
	std::unique_ptr<class AConvolver> convolver;

	// Buffer required for process()
	std::vector<float> processingBuffer;

	// GeneratingAudioComponent interface
	// size_t generateImpl(float* buffer, size_t count) override;
};
