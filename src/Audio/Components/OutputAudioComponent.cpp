#include "OutputAudioComponent.h"

OutputAudioComponent::OutputAudioComponent() :
	channels(0)
{
}

void OutputAudioComponent::init(float sampleRate, size_t channels, size_t bufferSize)
{
	AudioComponent::init(sampleRate, channels, bufferSize);
	outputBuffer.resize(bufferSize * channels);
	this->channels = channels;
}

const std::vector<float>& OutputAudioComponent::collectOutput()
{
	return outputBuffer;
}
