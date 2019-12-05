#include "AudioComponent.h"
#include "../Engine/EObject.h"
#include "ADelayLine.h"

AudioComponent::AudioComponent() :
	bAcceptsInput(false),
	bAcceptsOutput(false),
	bDirtyTransform(false),
	sampleRate(0.f),
	channels(0)
{
}

void AudioComponent::init(float sampleRate, size_t channels, size_t bufferSize)
{
	this->sampleRate = sampleRate;
	this->channels = channels;
	indirectInputBuffer.resize(bufferSize);
	for (const auto& input : inputs) input->init(sampleRate);
	for (const auto& output : outputs) output->init(sampleRate);
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
		auto ptr = output->dest.lock();
		ptr->otherTransformUpdated(*output, true);
	}
	for (const auto& input : inputs) {
		auto ptr = input->dest.lock();
		ptr->otherTransformUpdated(*input, false);
	}
}

void AudioComponent::updateVelocity(const mat::vec3& velocity)
{
	m_velocity = velocity;
	for (const auto& output : outputs) {
		auto ptr = output->dest.lock();
		mat::vec3 relPos = mat::normal(ptr->m_position - m_position);
		mat::vec3 relVel = m_velocity - ptr->m_velocity;
		output->velocity = mat::dot(relPos, relVel);
	}
	for (const auto& input : inputs) {
		auto ptr = input->dest.lock();
		mat::vec3 relPos = mat::normal(m_position - ptr->m_position);
		mat::vec3 relVel = ptr->m_velocity - m_velocity;
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
