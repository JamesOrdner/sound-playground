#pragma once

#include "OutputAudioComponent.h"
#include "../DSP/AInterpParameter.h"

class AMicrophone : public OutputAudioComponent
{
public:

	AMicrophone(const EObject* owner);

	// AudioComponent interface
	void init(float sampleRate) override;
	void transformUpdated() override;
	void otherTransformUpdated(const ADelayLine& connection, bool bInput) override;
	
	// OutputAudioComponent interface
	size_t processOutput(float* buffer, size_t n) override;

private:

	// Calculate the stereo gain of a given source (hardcoded X+ to the right)
	void calcStereoGain(const AudioComponent* source, float& gainL, float& gainR);

	// Left channel gain in a stereo setup. Right channel gain is calculated as 1 - gainL.
	AInterpParameter gainL;

	// Buffer used to copy data from input buffers
	std::vector<float> processingBuffer;
};
