#pragma once

#include "../AudioComponent.h"

class ASpeaker : public AudioComponent
{
public:

	ASpeaker();

	// AudioComponent interface
	size_t process(size_t n) override;

private:

	// Impulse response of the speaker
	std::vector<float> ir;

	// TEMPORARY direct convolution buffer
	std::vector<float> inputSignalBuffer;
	size_t inputSignalPtr;

	// Calculate the simple gain for a given destination
	float calcGain(const mat::vec3& dest);
};
