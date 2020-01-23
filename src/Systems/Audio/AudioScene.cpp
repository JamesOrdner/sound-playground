#include "AudioScene.h"
#include "AudioObject.h"
#include "AudioEngine.h"
#include "Components/AudioComponent.h"
#include "Components/AuralizingAudioComponent.h"
#include "Components/OutputAudioComponent.h"
#include "DSP/ADelayLine.h"

AudioScene::AudioScene(const SystemInterface* system, AudioEngine* audioEngine, const UScene* uscene) :
	SystemSceneInterface(system, uscene),
	audioEngine(audioEngine)
{
}

AudioScene::~AudioScene()
{
	audioObjects.clear();
}

void AudioScene::deleteSystemObject(const UObject* uobject)
{
	for (const auto& audioObject : audioObjects) {
		if (audioObject->uobject == uobject) {
			if (audioObject->audioComponent) {
				audioEngine->unregisterComponent(audioObject->audioComponent, this);
			}
			audioObjects.remove(audioObject);
			break;
		}
	}
}

void AudioScene::processSceneAudio(float* buffer, size_t frames)
{
	std::vector<size_t> outputProcessedCount(outputComponents.size());
	std::vector<size_t> auralizeProcessedCount(auralizingComponents.size());

	size_t i;
	while (true) {
		size_t maxRemaining = 0;
		i = 0;
		for (auto* c : outputComponents) {
			size_t processed = outputProcessedCount[i];
			processed += c->processOutput(buffer + processed, frames - processed);
			if (frames - processed > maxRemaining) maxRemaining = frames - processed;
			outputProcessedCount[i++] = processed;
		}

		i = 0;
		for (auto* c : auralizingComponents) {
			size_t processed = auralizeProcessedCount[i];
			processed += c->processIndirect(buffer + processed, frames - processed);
			if (frames - processed > maxRemaining) maxRemaining = frames - processed;
			auralizeProcessedCount[i++] = processed;
		}

		if (maxRemaining == 0) break;

		for (auto* c : components) {
			c->process(maxRemaining);
		}
	}
}

void AudioScene::connectAudioComponent(AudioComponent* component)
{
	// Setup delay lines
	for (AudioComponent* compOther : components) {
		// outputs
		if (component->bAcceptsOutput && compOther->bAcceptsInput) {
			auto output = std::make_shared<ADelayLine>(component, compOther);
			component->outputs.push_back(output);
			compOther->inputs.push_back(output);
			if (auto* gComp = dynamic_cast<GeneratingAudioComponent*>(component)) {
				output->genID = gComp->addConsumer();
			}
		}

		// inputs
		if (component->bAcceptsInput && compOther->bAcceptsOutput) {
			auto input = std::make_shared<ADelayLine>(compOther, component);
			compOther->outputs.push_back(input);
			component->inputs.push_back(input);
		}
	}

	// If OutputAudioComponent, setup indirect connections to this object
	if (auto* oComp = dynamic_cast<OutputAudioComponent*>(component)) {
		outputComponents.push_back(oComp);
		for (AudioComponent* compOther : components) {
			if (auto* aComp = dynamic_cast<AuralizingAudioComponent*>(compOther)) {
				IndirectSend* sendPtr = aComp->registerIndirectReceiver(oComp);
				oComp->registerIndirectSend(sendPtr);
			}
		}
	}

	// If AuralizingAudioComponent, setup indirect connections from this object
	if (auto* aComp = dynamic_cast<AuralizingAudioComponent*>(component)) {
		auralizingComponents.push_back(aComp);
		for (AudioComponent* compOther : components) {
			if (auto* oComp = dynamic_cast<OutputAudioComponent*>(compOther)) {
				IndirectSend* sendPtr = aComp->registerIndirectReceiver(oComp);
				oComp->registerIndirectSend(sendPtr);
			}
		}
	}

	// TODO: Sort by dependency for performance
	components.push_back(component);
}

void AudioScene::disconnectAudioComponent(AudioComponent* component)
{
	for (const auto& input : component->inputs) {
		input->source->outputs.remove(input);
	}

	for (const auto& output : component->outputs) {
		output->dest->inputs.remove(output);
		if (auto* gComp = dynamic_cast<GeneratingAudioComponent*>(component)) {
			gComp->removeConsumer(output->genID);
		}
	}

	components.remove(component);
	outputComponents.remove(dynamic_cast<OutputAudioComponent*>(component));
	auralizingComponents.remove(dynamic_cast<AuralizingAudioComponent*>(component));
}

size_t AudioScene::registeredComponentCount() const
{
	return components.size();
}

SystemObjectInterface* AudioScene::addSystemObject(SystemObjectInterface* object)
{
	audioObjects.emplace_back(static_cast<AudioObject*>(object));
	return object;
}

AudioComponent* AudioScene::addAudioComponentToObject(std::unique_ptr<class AudioComponent> component, AudioObject* object)
{
	object->audioComponent = component.get();
	audioEngine->registerComponent(std::move(component), this);
	return object->audioComponent;
}
