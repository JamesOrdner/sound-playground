#pragma once

#include "AuralizingAudioComponent.h"
#include "../DSP/AConvolver.h"
#include <memory>

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

	// Previous sample, saved for simple directional lpf
	float prev;

	// Calculate the simple gain for a given destination
	float calcGain(const mat::vec3& dest);
};
