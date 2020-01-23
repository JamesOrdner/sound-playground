#include "AudioScene.h"
#include "AudioObject.h"
#include "AudioEngine.h"
#include "Components/AudioComponent.h"
#include "Components/AuralizingAudioComponent.h"
#include "Components/OutputAudioComponent.h"
#include "DSP/ADelayLine.h"
#include <queue>

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
	if (outputComponents.empty()) return;

	std::vector<size_t> outputProcessedCount(outputComponents.size());
	std::vector<size_t> auralizeProcessedCount(auralizingComponents.size());

	std::queue<std::pair<ADelayLine*, size_t>> processQueue;
	for (auto* c : outputComponents) {
		for (const auto& input : c->inputs) {
			processQueue.push({ input.get(), frames });
		}
	}
	for (auto* c : auralizingComponents) {
		for (const auto& input : c->inputs) {
			processQueue.push({ input.get(), frames });
		}
	}

	while (!processQueue.empty()) {
		auto& dependency = processQueue.front();
		size_t n = dependency.first->source->process(dependency.first, dependency.second);
		if (n < dependency.second) {
			// some input(s) prevented this dependency from producing all required samples
			dependency.second -= n;
			for (const auto& sourceInput : dependency.first->source->inputs) {
				if (sourceInput->readable() < dependency.second) {
					// This input is holding us up, add to queue
					processQueue.push({ sourceInput.get(), dependency.second - sourceInput->readable() });
				}
			}
			processQueue.push(dependency);
		}
		processQueue.pop();
	}

	for (auto* c : outputComponents) c->processOutput(buffer, frames);
	for (auto* c : auralizingComponents) c->processIndirect(buffer, frames);
}

void AudioScene::connectAudioComponent(AudioComponent* component)
{
	for (const auto& output : component->outputs) {
		output->dest->inputs.push_back(output);
	}

	for (const auto& input : component->inputs) {
		input->source->outputs.push_back(input);

		// register this component with the other component's audio generator if necessary
		if (auto* gComp = dynamic_cast<GeneratingAudioComponent*>(input->source)) {
			input->genID = gComp->addConsumer();
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
	// setup delay lines to other components, without modifying the other components
	for (AudioComponent* compOther : components) {
		// outputs
		if (component->bAcceptsOutput && compOther->bAcceptsInput) {
			auto output = std::make_shared<ADelayLine>(component.get(), compOther);
			component->outputs.push_back(output);

			// register this output send with the audio generator if necessary
			if (auto* gComp = dynamic_cast<GeneratingAudioComponent*>(component.get())) {
				output->genID = gComp->addConsumer();
			}
		}

		// inputs
		if (component->bAcceptsInput && compOther->bAcceptsOutput) {
			auto input = std::make_shared<ADelayLine>(compOther, component.get());
			component->inputs.push_back(input);
		}
	}

	// attach component to AudioObject and pass to AudioEngine
	object->audioComponent = component.get();
	audioEngine->registerComponent(std::move(component), this);
	return object->audioComponent;
}
