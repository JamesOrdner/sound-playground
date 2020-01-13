#include "AuralizingAudioComponent.h"
#include "OutputAudioComponent.h"

AuralizingAudioComponent::AuralizingAudioComponent()
{
	genID = GeneratingAudioComponent::addConsumer();
}

AuralizingAudioComponent::~AuralizingAudioComponent()
{
	GeneratingAudioComponent::removeConsumer(genID);
}

void AuralizingAudioComponent::transformUpdated()
{
	AudioComponent::transformUpdated();
	for (auto& send : indirectReceivers) send->auralize();
}

void AuralizingAudioComponent::init(float sampleRate)
{
	AudioComponent::init(sampleRate);
	processingBuffer.resize(512); // TEMP
	for (auto& receiver : indirectReceivers) {
		for (auto& convolver : receiver->convolvers) {
			convolver.init(sampleRate);
		}
	}
}

void AuralizingAudioComponent::deinit()
{
	AudioComponent::deinit();
	processingBuffer.clear();
	for (auto& receiver : indirectReceivers) {
		for (auto& convolver : receiver->convolvers) {
			convolver.deinit();
		}
	}
}

IndirectSend* AuralizingAudioComponent::registerIndirectReceiver(OutputAudioComponent* receiver)
{
	// Check for existing entry
	for (auto& indirectReceiver : indirectReceivers) {
		if (indirectReceiver->receiver == receiver) return indirectReceiver.get();
	}

	auto send = std::make_unique<IndirectSend>();
	send->sender = this;
	send->receiver = receiver;

	for (size_t i = 0; i < 2; i++) { // TEMP
		std::vector<float> v(i * 100);
		v.push_back(0.3f);
		send->indirectIRs.push_back(v);
		AConvolver& convolver = send->convolvers.emplace_back();
		convolver.setIR(v);
		if (bInitialized) convolver.init(sampleRate);
	}

	IndirectSend* sendPtr = send.get();
	indirectReceivers.push_front(std::move(send));
	return sendPtr;
}

void AuralizingAudioComponent::unregisterIndirectReceiver(const OutputAudioComponent* receiver)
{
	for (auto& indirectReceiver : indirectReceivers) {
		if (indirectReceiver->receiver == receiver) {
			indirectReceivers.remove(indirectReceiver);
			break;
		}
	}
}

size_t AuralizingAudioComponent::processIndirect(float* buffer, size_t n)
{
	size_t available = GeneratingAudioComponent::readable(genID);
	if (available < n) GeneratingAudioComponent::generate(n - available);
	n = GeneratingAudioComponent::readGenerated(genID, processingBuffer.data(), n);

	for (auto& send : indirectReceivers) {
		size_t channels = send->convolvers.size();
		for (size_t ch = 0; ch < channels; ch++) {
			send->convolvers[ch].process(processingBuffer.data(), processingBuffer.data(), n);
			for (size_t i = 0; i < n; i++) buffer[i * channels + ch] += processingBuffer[i];
		}
	}

	return n;
}
