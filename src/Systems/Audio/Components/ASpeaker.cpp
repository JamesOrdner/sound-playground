#include "ASpeaker.h"
#include "../DSP/ADelayLine.h"
#include "../DSP/AConvolver.h"
#include "../DSP/AInterpParameter.h"

ASpeaker::ASpeaker() : sinGeneratorPhase(0)
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

void ASpeaker::initDelayLineData(ADelayLine* delayline, float sampleRate, bool bIsSource)
{
	AInterpParameter* gain = new AInterpParameter(0.f, 0.01f);
	gain->sampleRate = sampleRate;
	delayline->sourceData = gain;
}

void ASpeaker::deinitDelayLineData(ADelayLine* delayline, bool bIsSource)
{
	delete delayline->sourceData;
}

size_t ASpeaker::process(ADelayLine* output, size_t n)
{
	if (!output->writeable()) return 0;

	size_t available = readable(output->genID);
	if (available < n) generate(n - available);

	// read generated buffer
	size_t count = peekGenerated(output->genID, processingBuffer.data(), n);
	n = count;

	// process raw generated samples
	AInterpParameter* gain = static_cast<AInterpParameter*>(output->sourceData);
	gain->target = 1.f / mat::dist(position, output->dest->position) * 0.2f;
	float initialGain = gain->current; // save gain in order to rewind interp parameter if necessary

	for (size_t i = 0; i < count; i++) processingBuffer[i] *= gain->update();
	convolver->process(processingBuffer.data(), processingBuffer.data(), count);

	// write processed generated samples
	size_t written = output->write(processingBuffer.data(), count);
	
	// notify generator of how many generated samples we ended up using
	seekGenerated(output->genID, count);

	// rewind AInterpParameter if fewer samples were consumed than processed
	if (count < n) {
		gain->current = initialGain;
		gain->update(count);
	}

	return written;
}

size_t ASpeaker::generateImpl(float* buffer, size_t count)
{
	for (size_t i = 0; i < count; i++) {
		buffer[i] = sinf(sinGeneratorPhase++ * 2.f * mat::pi * 500.f / sampleRate); // sin wave
		// buffer[i] = static_cast<float>(rand()) / RAND_MAX * 2.f - 1.f; // white noise.
		buffer[i] *= 0.3f;
	}

	return count;
}
