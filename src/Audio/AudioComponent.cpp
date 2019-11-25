#include "AudioComponent.h"
#include "../Engine/EObject.h"
#include "ADelayLine.h"

void AudioComponent::init(size_t bufferSize, size_t channels)
{
	indirectInputBuffer.resize(bufferSize);
}

const mat::vec3& AudioComponent::getPosition()
{
	return owner.lock()->getPosition();
}
