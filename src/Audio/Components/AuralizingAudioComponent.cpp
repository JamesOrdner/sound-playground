#include "AuralizingAudioComponent.h"
#include "OutputAudioComponent.h"

AuralizingAudioComponent::AuralizingAudioComponent() :
	indirectBufferOffset(0)
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

void AuralizingAudioComponent::init(float sampleRate, size_t channels, size_t bufferSize)
{
	AudioComponent::init(sampleRate, channels, bufferSize);
	processIndirectBuffer.resize(bufferSize);
	for (auto& receiver : indirectReceivers) {
		for (auto& convolver : receiver->convolvers) {
			convolver.init(sampleRate);
		}
	}
}

void AuralizingAudioComponent::deinit()
{
	AudioComponent::deinit();
	processIndirectBuffer.clear();
	for (auto& receiver : indirectReceivers) {
		for (auto& convolver : receiver->convolvers) {
			convolver.deinit();
		}
	}
}

void AuralizingAudioComponent::preprocess()
{
	AudioComponent::preprocess();
	indirectBufferOffset = 0;
}

size_t AuralizingAudioComponent::indirectFramesProcessed()
{
	return indirectBufferOffset;
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
		v.push_back(0.4f);
		send->indirectIRs.push_back(v);
		send->convolvers.emplace_back();
		send->convolvers.back().setIR(v);
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

void AuralizingAudioComponent::processIndirect(size_t n)
{
	size_t available = GeneratingAudioComponent::readable(genID);
	if (available < n) GeneratingAudioComponent::generate(n - available);
	n = GeneratingAudioComponent::readGenerated(genID, processIndirectBuffer.data(), n);

	for (auto& send : indirectReceivers) {
		size_t channels = send->convolvers.size();
		float* dest = send->receiver->rawOutputBuffer() + indirectBufferOffset * channels;
		for (size_t ch = 0; ch < channels; ch++) {
			send->convolvers[ch].process(processIndirectBuffer.data(), processIndirectBuffer.data(), n);
			for (size_t i = 0; i < n; i++) dest[i * channels + ch] += processIndirectBuffer[i];
		}
	}

	indirectBufferOffset += n;
}
