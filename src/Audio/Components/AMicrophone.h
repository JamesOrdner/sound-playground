#pragma once

#include "../AudioOutputComponent.h"

class AMicrophone : public AudioOutputComponent
{
public:

	AMicrophone();

	// AudioComponent interface
	void init(size_t bufferSize, size_t channels) override;
	void deinit() override;
	void preprocess() override;
	size_t process(size_t n) override;

private:

	// Calculate the stereo gain of a given source (hardcoded X+ to the right)
	void calcStereoGain(const std::shared_ptr<AudioComponent>& source, float& gainL, float& gainR);

	// Points to the current location in the outputBuffer, reset each preprocess()
	size_t outputPtr;

	// Buffer used to copy data from input buffers
	float* inputBuffer;
};
