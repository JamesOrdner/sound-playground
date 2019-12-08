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
	AudioComponent::init(sampleRate, channels, bufferSize);
	convolver->init(sampleRate);
}

void ASpeaker::deinit()
{
	AudioComponent::deinit();
	convolver->deinit();
}

size_t t = 0;

size_t ASpeaker::process(size_t n)
{
	float out;
	for (size_t i = 0; i < n; i++) {
		for (const auto& output : outputs) if (!output->pushable()) return i;
		//float gen = sinf(t++ * 2.f * mat::pi * 500.f / sampleRate);
		float gen = static_cast<float>(rand()) / RAND_MAX * 2.f - 1.f;
		convolver->process(&out, &gen);
		for (const auto& output : outputs) output->push(&out);
	}
	
	return n;
}

float ASpeaker::calcGain(const mat::vec3& dest)
{
	using namespace mat;
	vec3 dir = normal(dest - position());
	return dot(dir, forward()) * 0.5f + 0.5f;
}
