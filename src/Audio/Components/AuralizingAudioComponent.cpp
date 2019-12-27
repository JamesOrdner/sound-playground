#include "AuralizingAudioComponent.h"
#include "OutputAudioComponent.h"

AuralizingAudioComponent::AuralizingAudioComponent() :
	indirectBufferOffset(0)
{
}

void AuralizingAudioComponent::transformUpdated()
{
	AudioComponent::transformUpdated();
	for (auto& send : indirectReceivers) send->auralize();
}

void AuralizingAudioComponent::preprocess()
{
	AudioComponent::preprocess();
	indirectBufferOffset = 0;
}

IndirectSend* AuralizingAudioComponent::registerIndirectReceiver(OutputAudioComponent* receiver)
{
	// Check for existing entry
	for (auto it = indirectReceivers.cbegin(); it != indirectReceivers.cend(); it++) {
		if ((*it)->receiver == receiver) return it->get();
	}

	auto send = std::make_unique<IndirectSend>(this, receiver);
	IndirectSend* sendPtr = send.get();
	indirectReceivers.push_front(std::move(send));
	return sendPtr;
}

void AuralizingAudioComponent::unregisterIndirectReceiver(const OutputAudioComponent* receiver)
{
	for (auto it = indirectReceivers.cbegin(); it != indirectReceivers.cend(); it++) {
		if ((*it)->receiver == receiver) {
			indirectReceivers.erase(it);
			break;
		}
	}
}

void AuralizingAudioComponent::processIndirect(const float* componentOutput, size_t n)
{
	for (auto& send : indirectReceivers) {
		float* dest = send->receiver->rawOutputBuffer() + indirectBufferOffset;
		send->convolver.process(dest, componentOutput, n);
	}

	indirectBufferOffset += n;
}
