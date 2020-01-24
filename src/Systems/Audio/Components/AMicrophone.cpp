#include "AMicrophone.h"
#include "../DSP/ADelayLine.h"

AMicrophone::AMicrophone() :
	processingBuffer(512) // TEMP
{
	bAcceptsInput = true;
	bAcceptsOutput = false;
}

void AMicrophone::init(float sampleRate)
{
	OutputAudioComponent::init(sampleRate);
}

void AMicrophone::transformUpdated()
{
	OutputAudioComponent::transformUpdated();
}

void AMicrophone::otherTransformUpdated(const ADelayLine& connection, bool bInput)
{
	OutputAudioComponent::otherTransformUpdated(connection, bInput);
}

size_t AMicrophone::processOutput(float* buffer, size_t n)
{
	for (const auto& input : inputs) {
		n = input->read(processingBuffer.data(), n);
		size_t bIdx = 0;
		for (size_t i = 0; i < n; i++) {
			// This is all hardcoded for stereo, needs to be changed eventually
			buffer[bIdx++] += processingBuffer[i];
			buffer[bIdx++] += processingBuffer[i];
		}
	}

	return n;
}

void AMicrophone::calcStereoGain(const AudioComponent* source, float& gainL, float& gainR)
{
	mat::vec3 dir = source->position - position;
	float angle = atanf(dir.x / fabsf(dir.z)) * 0.5f + 0.25f * mat::pi;
	gainL = std::isnan(angle) ? 0.f : cosf(angle);
	gainR = std::isnan(angle) ? 0.f : sinf(angle);
}
