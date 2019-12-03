#include "AMicrophone.h"
#include "../ADelayLine.h"

AMicrophone::AMicrophone() :
	outputPtr(0),
	inputBuffer(nullptr)
{
	bAcceptsInput = true;
	bAcceptsOutput = false;
}

void AMicrophone::init(float sampleRate, size_t channels, size_t bufferSize)
{
	AudioOutputComponent::init(sampleRate, channels, bufferSize);
	inputBuffer = new float[bufferSize];
}

void AMicrophone::deinit()
{
	delete[] inputBuffer;
}

void AMicrophone::preprocess()
{
	outputPtr = 0;
	std::fill(outputBuffer.begin(), outputBuffer.end(), 0.f);
}

size_t AMicrophone::process(size_t n)
{
	size_t p = pullCount();
	if (p < n) n = p;
	for (const auto& input : inputs) {
		// This is all hardcoded for stereo, needs to be changed eventually
		float gL, gR;
		calcStereoGain(input->source.lock(), gL, gR);
		input->buffer.read(inputBuffer, n);
		for (size_t i = 0; i < n; i++) {
			outputBuffer[outputPtr + i * 2] += inputBuffer[i] * gL;
			outputBuffer[outputPtr + i * 2 + 1] += inputBuffer[i] * gR;
		}
	}
	outputPtr += n * channels;
	return n;
}

void AMicrophone::calcStereoGain(const std::shared_ptr<AudioComponent>& source, float& gainL, float& gainR)
{
	mat::vec3 dir = source->position() - position();
	float angle = atanf(dir.x / fabsf(dir.z)) * 0.5f + 0.25f * mat::pi;
	gainL = cosf(angle);
	gainR = sinf(angle);
}
