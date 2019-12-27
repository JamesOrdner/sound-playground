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
}

void ASpeaker::deinit()
{
	AuralizingAudioComponent::deinit();
	convolver->deinit();
}

size_t t = 0;

size_t ASpeaker::process(size_t n)
{
	float out;
	for (size_t i = 0; i < n; i++) {
		for (const auto& output : outputs) if (!output->writeable()) return i;
		float gen = sinf(t++ * 2.f * mat::pi * 500.f / sampleRate); // sin wave
		// float gen = static_cast<float>(rand()) / RAND_MAX * 2.f - 1.f; // white noise
		convolver->process(&out, &gen);
		for (const auto& output : outputs) output->write(&out);
		processIndirect(&gen, 1);
	}
	
	return n;
}

float ASpeaker::calcGain(const mat::vec3& dest)
{
	using namespace mat;
	vec3 dir = normal(dest - position());
	return dot(dir, forward()) * 0.5f + 0.5f;
}
