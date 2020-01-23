#include "ASpeaker.h"
#include "../DSP/ADelayLine.h"
#include "../DSP/AConvolver.h"

ASpeaker::ASpeaker()
{
	bAcceptsInput = false;
	bAcceptsOutput = true;
	
	convolver = std::make_unique<AConvolver>("res/sound/auratone_8192.wav");
}

//void ASpeaker::init(float sampleRate)
//{
//	AuralizingAudioComponent::init(sampleRate);
//	convolver->init(sampleRate);
//	processingBuffer.clear();
//	processingBuffer.resize(512); // TEMP
//}

//void ASpeaker::deinit()
//{
//	AuralizingAudioComponent::deinit();
//	convolver->deinit();
//}

size_t ASpeaker::process(size_t n)
{
	//for (const auto& output : outputs) {
	//	if (!output->writeable()) continue;
	//	size_t available = GeneratingAudioComponent::readable(output->genID);
	//	if (available < n) GeneratingAudioComponent::generate(n - available);
	//	size_t count = GeneratingAudioComponent::peekGenerated(output->genID, processingBuffer.data(), n);
	//	convolver->process(processingBuffer.data(), processingBuffer.data(), count);
	//	size_t written = output->write(processingBuffer.data(), count);
	//	GeneratingAudioComponent::seekGenerated(output->genID, written);
	//	if (n > written) n = written; // TEMP
	//}

	size_t written = 0;
	while (written < n) {
		for (const auto& output : outputs) {
			if (!output->writeable()) return written;
		}

		float gen = static_cast<float>(rand()) / RAND_MAX * 2.f - 1.f;
		for (const auto& output : outputs) {
			float gain = 1.f / mat::dist(position, output->dest->position) * 0.2f;
			float final = gen * gain;
			output->write(&final, 1);
		}

		written++;
	}
	
	return n;
}

//size_t t = 0;
//size_t ASpeaker::generateImpl(float* buffer, size_t count)
//{
//	for (size_t i = 0; i < count; i++) {
//		buffer[i] = sinf(t++ * 2.f * mat::pi * 500.f / sampleRate); // sin wave
//		// buffer[i] = static_cast<float>(rand()) / RAND_MAX * 2.f - 1.f; // white noise.
//		buffer[i] *= 0.3f;
//	}
//
//	return count;
//}
