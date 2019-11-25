#include "AudioOutputComponent.h"

void AudioOutputComponent::init(size_t bufferSize, size_t channels)
{
	AudioComponent::init(bufferSize, channels);
	outputBuffer.resize(bufferSize * channels);
	this->channels = channels;
}

const std::vector<float>& AudioOutputComponent::collectOutput()
{
	return outputBuffer;
}
