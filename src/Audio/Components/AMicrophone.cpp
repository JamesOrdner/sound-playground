#include "AMicrophone.h"
#include "../ADelayLine.h"

void AMicrophone::process(size_t n)
{
	std::vector<float> inBuffer(n);
	for (const auto& input : inputs) {
		input->buffer.read(&inBuffer[0], n);
		for (size_t i = 0; i < n; i++) {
			for (size_t ch = 0; ch < channels; ch++) {
				outputBuffer[i * channels + ch] = inBuffer[i];
			}
		}
	}

	// TEMPORARY
	for (size_t i = 0; i < n; i++) {
		for (size_t ch = 0; ch < channels; ch++) {
			float s = static_cast<float>(rand()) / RAND_MAX * 2.f - 1.f;
			outputBuffer[i * channels + ch] = s;
		}
	}
}
