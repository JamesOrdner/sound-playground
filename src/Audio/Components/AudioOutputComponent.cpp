#include "AudioOutputComponent.h"

void AudioOutputComponent::init(float sampleRate, size_t channels, size_t bufferSize)
{
	AudioComponent::init(sampleRate, channels, bufferSize);
	outputBuffer.resize(bufferSize * channels);
}

const std::vector<float>& AudioOutputComponent::collectOutput()
{
	return outputBuffer;
}
