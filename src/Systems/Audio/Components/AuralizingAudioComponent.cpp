#include "AuralizingAudioComponent.h"
#include "OutputAudioComponent.h"

IndirectSend::IndirectSend(AuralizingAudioComponent* sender, OutputAudioComponent* receiver) :
	sender(sender),
	receiver(receiver)
{
	indirectIRs.emplace_back();
	indirectIRs.emplace_back();

	convolvers.emplace_back();
	convolvers.emplace_back();
}

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
	for (auto& send : indirectSends) send->auralize();
}

void AuralizingAudioComponent::init(float sampleRate)
{
	AudioComponent::init(sampleRate);
	processingBuffer.resize(512); // TEMP
	for (const auto& indirectSend : indirectSends) {
		for (auto& convolver : indirectSend->convolvers) {
			convolver.init(sampleRate);
		}
	}
}

void AuralizingAudioComponent::deinit()
{
	AudioComponent::deinit();
	processingBuffer.clear();
	for (const auto& indirectSend : indirectSends) {
		for (auto& convolver : indirectSend->convolvers) {
			convolver.deinit();
		}
	}
}

size_t AuralizingAudioComponent::processIndirect(float* buffer, size_t n)
{
	size_t available = GeneratingAudioComponent::readable(genID);
	if (available < n) GeneratingAudioComponent::generate(n - available);
	n = GeneratingAudioComponent::readGenerated(genID, processingBuffer.data(), n);

	for (const auto& indirectSend : indirectSends) {
		size_t channels = indirectSend->convolvers.size();
		for (size_t ch = 0; ch < channels; ch++) {
			indirectSend->convolvers[ch].process(processingBuffer.data(), processingBuffer.data(), n);
			// for (size_t i = 0; i < n; i++) buffer[i * channels + ch] += processingBuffer[i];
		}
	}

	return n;
}
