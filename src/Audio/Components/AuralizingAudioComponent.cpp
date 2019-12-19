#include "AuralizingAudioComponent.h"
#include "OutputAudioComponent.h"

void AuralizingAudioComponent::init(float sampleRate, size_t channels, size_t bufferSize)
{
	AudioComponent::init(sampleRate, channels, bufferSize);
	indirectBuffer.init(bufferSize, bufferSize);
	processIndirectBuffer.reserve(bufferSize);
}

void AuralizingAudioComponent::registerReceiver(OutputAudioComponent* receiver)
{
	// Check for existing entry
	for (auto it = receivers.cbegin(); it != receivers.cend(); it++) {
		if (it->receiver == receiver) return;
	}

	AuralSend send;
	send.receiver = receiver;
	receivers.push_back(send);
}

void AuralizingAudioComponent::unregisterReceiver(const OutputAudioComponent* receiver)
{
	for (auto it = receivers.cbegin(); it != receivers.cend(); it++) {
		if (it->receiver == receiver) {
			receivers.erase(it);
			break;
		}
	}
}

void AuralizingAudioComponent::processIndirect()
{
	size_t n = indirectBuffer.readable();
	if (n != processIndirectBuffer.capacity()) {
		printf("WARNING: AuralizingAudioComponent::processIndirect(): buffer size mismatch!\n");
		return;
	}

	indirectBuffer.read(processIndirectBuffer.data(), n);
	for (auto it = receivers.begin(); it != receivers.end(); it++) {
		float* out = it->receiver->outputBuffer.data();
		size_t ptr = 0;
		for (size_t i = 0; i < n; i++) {
			for (size_t ch = 0; ch < it->convolver.size(); ch++) {
				it->convolver[ch].process(out + ptr++, processIndirectBuffer.data() + i);
			}
		}
	}
}
