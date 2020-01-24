#pragma once

#include "OutputAudioComponent.h"

class AMicrophone : public OutputAudioComponent
{
public:

	AMicrophone();

	// AudioComponent interface
	void init(float sampleRate) override;
	void transformUpdated() override;
	void otherTransformUpdated(const ADelayLine& connection, bool bInput) override;
	
	// OutputAudioComponent interface
	size_t processOutput(float* buffer, size_t n) override;

private:

	// Calculate the stereo gain of a given source (hardcoded X+ to the right)
	void calcStereoGain(const AudioComponent* source, float& gainL, float& gainR);

	// Buffer used to copy data from input buffers
	std::vector<float> processingBuffer;
};
