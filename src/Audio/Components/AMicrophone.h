#pragma once

#include "OutputAudioComponent.h"
#include "../DSP/AInterpParameter.h"

class AMicrophone : public OutputAudioComponent
{
public:

	AMicrophone();

	// AudioComponent interface
	void init(float sampleRate, size_t channels, size_t bufferSize) override;
	void deinit() override;
	void transformUpdated() override;
	void otherTransformUpdated(const ADelayLine& connection, bool bInput) override;
	void preprocess() override;
	size_t process(size_t n) override;

private:

	// Calculate the stereo gain of a given source (hardcoded X+ to the right)
	void calcStereoGain(const AudioComponent& source, float& gainL, float& gainR);

	// Left channel gain in a stereo setup. Right channel gain is calculated as 1 - gainL.
	AInterpParameter gainL;

	// Points to the current location in the outputBuffer, reset each preprocess()
	size_t outputPtr;

	// Buffer used to copy data from input buffers
	float* inputBuffer;
};
