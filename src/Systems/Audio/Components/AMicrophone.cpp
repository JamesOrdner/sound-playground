#include "AMicrophone.h"
#include "../DSP/ADelayLine.h"

AMicrophone::AMicrophone(const EObject* owner) :
	OutputAudioComponent(owner),
	processingBuffer(512), // TEMP
	gainL(0.5)
{
	bAcceptsInput = true;
	bAcceptsOutput = false;
}

void AMicrophone::init(float sampleRate)
{
	OutputAudioComponent::init(sampleRate);
	gainL.sampleRate = sampleRate;
}

void AMicrophone::transformUpdated()
{
	OutputAudioComponent::transformUpdated();
	float gL, gR;
	calcStereoGain(inputs.front()->source, gL, gR);
	gainL.target = gL;
}

void AMicrophone::otherTransformUpdated(const ADelayLine& connection, bool bInput)
{
	OutputAudioComponent::otherTransformUpdated(connection, bInput);
	float gL, gR;
	calcStereoGain(connection.source, gL, gR);
	gainL.target = gL;
}

size_t AMicrophone::processOutput(float* buffer, size_t n)
{
	for (const auto& input : inputs) {
		n = input->read(processingBuffer.data(), n);
		size_t bIdx = 0;
		for (size_t i = 0; i < n; i++) {
			// This is all hardcoded for stereo, needs to be changed eventually
			float gL = gainL.update();
			float gR = 1.f - gL;
			buffer[bIdx++] += processingBuffer[i];// * gL * 0.5f;
			buffer[bIdx++] += processingBuffer[i];// * gR * 0.5f;
		}
	}

	return n;
}

void AMicrophone::calcStereoGain(const AudioComponent* source, float& gainL, float& gainR)
{
	mat::vec3 dir = source->componentPosition() - componentPosition();
	float angle = atanf(dir.x / fabsf(dir.z)) * 0.5f + 0.25f * mat::pi;
	gainL = std::isnan(angle) ? 0.f : cosf(angle);
	gainR = std::isnan(angle) ? 0.f : sinf(angle);
}
