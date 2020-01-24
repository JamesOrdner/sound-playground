#include "ASpeaker.h"
#include "../DSP/ADelayLine.h"
#include "../DSP/AConvolver.h"

ASpeaker::ASpeaker()
{
	bAcceptsInput = false;
	bAcceptsOutput = true;
	
	convolver = std::make_unique<AConvolver>("res/sound/auratone_8192.wav");
}

void ASpeaker::init(float sampleRate)
{
	AuralizingAudioComponent::init(sampleRate);
	convolver->init(sampleRate);
	processingBuffer.clear();
	processingBuffer.resize(512); // TEMP
}

void ASpeaker::deinit()
{
	AuralizingAudioComponent::deinit();
	convolver->deinit();
}

size_t ASpeaker::process(ADelayLine* output, size_t n)
{
	if (!output->writeable()) return 0;

	size_t available = GeneratingAudioComponent::readable(output->genID);
	if (available < n) GeneratingAudioComponent::generate(n - available);

	size_t count = GeneratingAudioComponent::peekGenerated(output->genID, processingBuffer.data(), n);
	float gain = 1.f / mat::dist(position, output->dest->position) * 0.2f;
	for (size_t i = 0; i < count; i++) processingBuffer[i] *= gain;
	convolver->process(processingBuffer.data(), processingBuffer.data(), count);

	size_t written = output->write(processingBuffer.data(), count);
	GeneratingAudioComponent::seekGenerated(output->genID, count);
	return written;
}

size_t t = 0;
size_t ASpeaker::generateImpl(float* buffer, size_t count)
{
	for (size_t i = 0; i < count; i++) {
		buffer[i] = sinf(t++ * 2.f * mat::pi * 500.f / sampleRate); // sin wave
		// buffer[i] = static_cast<float>(rand()) / RAND_MAX * 2.f - 1.f; // white noise.
		buffer[i] *= 0.3f;
	}

	return count;
}
