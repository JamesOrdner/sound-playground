#include "AudioComponent.h"
#include "../DSP/ADelayLine.h"
#include "../AudioObject.h"

AudioComponent::AudioComponent() :
	bAcceptsInput(false),
	bAcceptsOutput(false),
	sampleRate(0.f),
	bInitialized(false)
{
}

AudioComponent::~AudioComponent()
{
}

void AudioComponent::init(float sampleRate)
{
	for (const auto& input : inputs) input->init(sampleRate);
	for (const auto& output : outputs) output->init(sampleRate);
	this->sampleRate = sampleRate;
	bInitialized = true;
}

void AudioComponent::deinit()
{
	bInitialized = false;
}

mat::vec3 AudioComponent::forward() const
{
	return mat::forward(rotation);
}

void AudioComponent::transformUpdated()
{
	for (const auto& output : outputs) {
		output->dest->otherTransformUpdated(*output, true);
	}
	for (const auto& input : inputs) {
		input->dest->otherTransformUpdated(*input, false);
	}
}

void AudioComponent::velocityUpdated()
{
	for (const auto& output : outputs) {
		mat::vec3 relPos = mat::normal(output->dest->position - position);
		mat::vec3 relVel = velocity - output->dest->velocity;
		output->velocity = mat::dot(relPos, relVel);
	}
	for (const auto& input : inputs) {
		mat::vec3 relPos = mat::normal(position - input->dest->position);
		mat::vec3 relVel = input->dest->velocity - velocity;
		input->velocity = mat::dot(relPos, relVel);
	}
}
