#include "AMicrophone.h"
#include "../DSP/ADelayLine.h"

AMicrophone::AMicrophone() :
	outputPtr(0),
	inputBuffer(nullptr),
	gainL(0.5)
{
	bAcceptsInput = true;
	bAcceptsOutput = false;
}

void AMicrophone::init(float sampleRate, size_t channels, size_t bufferSize)
{
	OutputAudioComponent::init(sampleRate, channels, bufferSize);
	inputBuffer = new float[bufferSize];
	gainL.sampleRate = sampleRate;
}

void AMicrophone::deinit()
{
	OutputAudioComponent::deinit();
	delete[] inputBuffer;
}

void AMicrophone::transformUpdated()
{
	OutputAudioComponent::transformUpdated();
	float gL, gR;
	calcStereoGain(*inputs.front()->source.lock(), gL, gR);
	gainL.target = gL;
}

void AMicrophone::otherTransformUpdated(const ADelayLine& connection, bool bInput)
{
	OutputAudioComponent::otherTransformUpdated(connection, bInput);
	float gL, gR;
	calcStereoGain(*connection.source.lock(), gL, gR);
	gainL.target = gL;
}

void AMicrophone::preprocess()
{
	OutputAudioComponent::preprocess();
	outputPtr = 0;
}

size_t AMicrophone::process(size_t n)
{
	if (outputPtr + n * channels >= outputBuffer.size()) {
		n = (outputBuffer.size() - outputPtr) / channels;
	}

	for (const auto& input : inputs) {
		n = input->read(inputBuffer, n);
		for (size_t i = 0; i < n; i++) {
			// This is all hardcoded for stereo, needs to be changed eventually
			float gL = gainL.update();
			float gR = 1.f - gL;
			outputBuffer[outputPtr++] += inputBuffer[i] * gL * 0.5f;
			outputBuffer[outputPtr++] += inputBuffer[i] * gR * 0.5f;
		}
	}

	return n;
}

void AMicrophone::calcStereoGain(const AudioComponent& source, float& gainL, float& gainR)
{
	mat::vec3 dir = source.position() - position();
	float angle = atanf(dir.x / fabsf(dir.z)) * 0.5f + 0.25f * mat::pi;
	gainL = cosf(angle);
	gainR = sinf(angle);
}
