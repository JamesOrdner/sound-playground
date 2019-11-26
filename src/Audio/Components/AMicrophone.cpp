#include "AMicrophone.h"
#include "../ADelayLine.h"

AMicrophone::AMicrophone() :
	outputPtr(0)
{
	bAcceptsInput = true;
	bAcceptsOutput = false;
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
	std::vector<float> inBuffer(n);
	for (const auto& input : inputs) {
		input->buffer.read(&inBuffer[0], n);
		for (size_t i = 0; i < n; i++) {
			for (size_t ch = 0; ch < channels; ch++) {
				outputBuffer[outputPtr++] = inBuffer[i];
			}
		}
	}
	return n;
}
