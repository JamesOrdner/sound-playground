#include "AudioComponent.h"
#include "../Engine/EObject.h"
#include "ADelayLine.h"

AudioComponent::AudioComponent() :
	bAcceptsInput(false),
	bAcceptsOutput(false),
	bDirtyTransform(false)
{
}

void AudioComponent::init(size_t bufferSize, size_t channels)
{
	indirectInputBuffer.resize(bufferSize);
	for (const auto& input : inputs) input->buffer.init(bufferSize);
	for (const auto& output : outputs) output->buffer.init(bufferSize);
}

size_t AudioComponent::shortestInput()
{
	size_t shortest = SIZE_MAX;
	for (const auto& input : inputs) {
		size_t size = input->buffer.size();
		if (size < shortest) shortest = size;
	}
	return shortest;
}

const mat::vec3& AudioComponent::position()
{
	return m_position;
}

const mat::vec3& AudioComponent::forward()
{
	return m_forward;
}

void AudioComponent::transformUpdated()
{
	bDirtyTransform = true;
}

size_t AudioComponent::pullCount()
{
	size_t shortest = SIZE_MAX;
	for (const auto& input : inputs) {
		size_t count = input->buffer.pullCount();
		if (count < shortest) shortest = count;
	}
	return shortest;
}

size_t AudioComponent::pushCount()
{
	size_t shortest = SIZE_MAX;
	for (const auto& output : outputs) {
		size_t count = output->buffer.pushCount();
		if (count < shortest) shortest = count;
	}
	return shortest;
}
