#include "AudioComponent.h"
#include "../DSP/ADelayLine.h"

AudioComponent::AudioComponent() :
	bAcceptsInput(false),
	bAcceptsOutput(false),
	m_owner(nullptr),
	bDirtyTransform(false),
	sampleRate(0.f)
{
}

void AudioComponent::init(float sampleRate, size_t channels, size_t bufferSize)
{
	for (const auto& input : inputs) input->init(sampleRate);
	for (const auto& output : outputs) output->init(sampleRate);
	this->sampleRate = sampleRate;
}

const mat::vec3& AudioComponent::position() const
{
	return m_position;
}

const mat::vec3& AudioComponent::velocity() const
{
	return m_velocity;
}

const mat::vec3& AudioComponent::forward() const
{
	return m_forward;
}

void AudioComponent::transformUpdated()
{
	bDirtyTransform = true;
	for (const auto& output : outputs) {
		output->dest->otherTransformUpdated(*output, true);
	}
	for (const auto& input : inputs) {
		input->dest->otherTransformUpdated(*input, false);
	}
}

void AudioComponent::updateVelocity(const mat::vec3& velocity)
{
	m_velocity = velocity;
	for (const auto& output : outputs) {
		mat::vec3 relPos = mat::normal(output->dest->m_position - m_position);
		mat::vec3 relVel = m_velocity - output->dest->m_velocity;
		output->velocity = mat::dot(relPos, relVel);
	}
	for (const auto& input : inputs) {
		mat::vec3 relPos = mat::normal(m_position - input->dest->m_position);
		mat::vec3 relVel = input->dest->m_velocity - m_velocity;
		input->velocity = mat::dot(relPos, relVel);
	}
}

size_t AudioComponent::pullCount()
{
	size_t shortest = SIZE_MAX;
	for (const auto& input : inputs) {
		size_t count = input->readable();
		if (count < shortest) shortest = count;
	}
	return shortest;
}
