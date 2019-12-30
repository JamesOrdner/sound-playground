#include "ASpeaker.h"
#include "../../Engine/EObject.h"
#include "../DSP/ADelayLine.h"

ASpeaker::ASpeaker() :
	prev(0.f)
{
	bAcceptsInput = false;
	bAcceptsOutput = true;
	
	convolver = std::make_unique<AConvolver>("res/sound/auratone_8192.wav");
}

void ASpeaker::init(float sampleRate, size_t channels, size_t bufferSize)
{
	AuralizingAudioComponent::init(sampleRate, channels, bufferSize);
	convolver->init(sampleRate);
	workingBuffer.clear();
	workingBuffer.resize(bufferSize);
}

void ASpeaker::deinit()
{
	AuralizingAudioComponent::deinit();
	convolver->deinit();
}

size_t ASpeaker::process(size_t n)
{
	for (const auto& output : outputs) {
		if (!output->writeable()) continue;
		size_t available = GeneratingAudioComponent::readable(output->genID);
		if (available < n) GeneratingAudioComponent::generate(n - available);
		size_t count = GeneratingAudioComponent::peekGenerated(output->genID, workingBuffer.data(), n);
		convolver->process(workingBuffer.data(), workingBuffer.data(), count);
		size_t written = output->write(workingBuffer.data(), count);
		GeneratingAudioComponent::seekGenerated(output->genID, written);
		if (n > written) n = written; // TEMP
	}
	
	return n;
}

float ASpeaker::calcGain(const mat::vec3& dest)
{
	using namespace mat;
	vec3 dir = normal(dest - position());
	return dot(dir, forward()) * 0.5f + 0.5f;
}

size_t t = 0;
size_t ASpeaker::generateImpl(float* buffer, size_t count)
{
	for (size_t i = 0; i < count; i++) {
		buffer[i] = sinf(t++ * 2.f * mat::pi * 500.f / sampleRate); // sin wave
		// buffer[i] = static_cast<float>(rand()) / RAND_MAX * 2.f - 1.f; // white noise
	}

	return count;
}
