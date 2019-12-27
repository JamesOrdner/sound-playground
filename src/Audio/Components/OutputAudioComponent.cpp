#include "OutputAudioComponent.h"
#include "AuralizingAudioComponent.h"

OutputAudioComponent::OutputAudioComponent() :
	channels(0)
{
}

void OutputAudioComponent::registerIndirectSend(IndirectSend* send)
{
	indirectSends.remove(send); // don't allow duplicates
	indirectSends.push_front(send);
}

void OutputAudioComponent::unregisterIndirectSend(const IndirectSend* receiver)
{
	for (auto it = indirectSends.cbegin(); it != indirectSends.cend(); it++) {
		if (*it == receiver) {
			indirectSends.erase(it);
			return;
		}
	}
}

void OutputAudioComponent::init(float sampleRate, size_t channels, size_t bufferSize)
{
	AudioComponent::init(sampleRate, channels, bufferSize);
	outputBuffer.resize(bufferSize * channels);
	this->channels = channels;
}

void OutputAudioComponent::transformUpdated()
{
	AudioComponent::transformUpdated();
	for (auto send : indirectSends) send->auralize();
}

void OutputAudioComponent::preprocess()
{
	AudioComponent::preprocess();
	std::fill(outputBuffer.begin(), outputBuffer.end(), 0.f);
}

float* OutputAudioComponent::rawOutputBuffer()
{
	return outputBuffer.data();
}
