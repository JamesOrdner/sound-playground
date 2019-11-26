#include "AMicrophone.h"
#include "../ADelayLine.h"

AMicrophone::AMicrophone()
{
	bAcceptsInput = true;
	bAcceptsOutput = false;
}

void AMicrophone::init(size_t bufferSize, size_t channels)
{
	AudioOutputComponent::init(bufferSize, channels);
	inputBuffer = new float[bufferSize];
}

void AMicrophone::deinit()
{
	delete[] inputBuffer;
}

void AMicrophone::preprocess()
{
	outputPtr = 0;
}

size_t AMicrophone::process(size_t n)
{
	if (n == 0) return n;

	size_t p = pullCount();
	if (p < n) n = p;
	for (const auto& input : inputs) {
		input->buffer.read(inputBuffer, n);
		for (size_t i = 0; i < n; i++) {
			for (size_t ch = 0; ch < channels; ch++) {
				outputBuffer[outputPtr++] = inputBuffer[i];
			}
		}
	}
	return n;
}
