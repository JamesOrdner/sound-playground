#include "AudioComponent.h"
#include "../DSP/ADelayLine.h"
#include "../../Objects/EObject.h"
#include "../../Managers/StateManager.h"

AudioComponent::AudioComponent(const EObject* owner) :
	bAcceptsInput(false),
	bAcceptsOutput(false),
	sampleRate(0.f),
	bInitialized(false)
{
	auto& stateManager = StateManager::instance();

	audioObserverIDs.push_back(
		stateManager.registerAudioObserver(
			owner,
			StateManager::EventType::PositionUpdated,
			[this](const StateManager::EventData& data) {
				position = std::get<mat::vec3>(data);
				transformUpdated();
			}
		)
	);

	audioObserverIDs.push_back(
		stateManager.registerAudioObserver(
			owner,
			StateManager::EventType::VelocityUpdated,
			[this](const StateManager::EventData& data) {
				velocity = std::get<mat::vec3>(data);
				velocityUpdated();
			}
		)
	);
}

AudioComponent::~AudioComponent()
{
	for (auto id : audioObserverIDs) {
		StateManager::instance().unregisterAudioObserver(id);
	}
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

const mat::vec3& AudioComponent::componentPosition() const
{
	return position;
}

const mat::vec3& AudioComponent::componentVelocity() const
{
	return velocity;
}

mat::vec3 AudioComponent::forward() const
{
	return mat::vec3{ 0, 0, 1 };
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
		mat::vec3 relPos = mat::normal(output->dest->componentPosition() - position);
		mat::vec3 relVel = velocity - output->dest->componentVelocity();
		output->velocity = mat::dot(relPos, relVel);
	}
	for (const auto& input : inputs) {
		mat::vec3 relPos = mat::normal(position - input->dest->componentPosition());
		mat::vec3 relVel = input->dest->componentVelocity() - velocity;
		input->velocity = mat::dot(relPos, relVel);
	}
}
