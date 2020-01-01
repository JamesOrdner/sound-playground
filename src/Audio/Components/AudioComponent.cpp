#include "AudioComponent.h"
#include "../DSP/ADelayLine.h"
#include "../../Engine/EObject.h"

AudioComponent::AudioComponent() :
	bAcceptsInput(false),
	bAcceptsOutput(false),
	sampleRate(0.f),
	bInitialized(false),
	owner(nullptr)
{
}

AudioComponent::~AudioComponent() = default;

void AudioComponent::init(float sampleRate, size_t channels, size_t bufferSize)
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

void AudioComponent::transformUpdated()
{
	for (const auto& output : outputs) {
		output->dest->otherTransformUpdated(*output, true);
	}
	for (const auto& input : inputs) {
		input->dest->otherTransformUpdated(*input, false);
	}
}

const mat::vec3& AudioComponent::position() const
{
	return owner->position();
}

const mat::vec3& AudioComponent::velocity() const
{
	return owner->velocity();
}

mat::vec3 AudioComponent::forward() const
{
	return owner->forward();
}

void AudioComponent::updateVelocity(const mat::vec3& velocity)
{
	for (const auto& output : outputs) {
		mat::vec3 relPos = mat::normal(output->dest->position() - position());
		mat::vec3 relVel = velocity - output->dest->velocity();
		output->velocity = mat::dot(relPos, relVel);
	}
	for (const auto& input : inputs) {
		mat::vec3 relPos = mat::normal(position() - input->dest->position());
		mat::vec3 relVel = input->dest->velocity() - velocity;
		input->velocity = mat::dot(relPos, relVel);
	}
}
