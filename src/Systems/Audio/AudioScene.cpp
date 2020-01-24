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
	// direct connections
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

	// indirect connections
	if (auto* outComp = dynamic_cast<OutputAudioComponent*>(component)) {
		outputComponents.push_back(outComp);
		for (const auto& indirectSend : outComp->indirectSends) {
			indirectSend->sender->indirectSends.push_back(indirectSend);
		}
	}

	if (auto* auralComp = dynamic_cast<AuralizingAudioComponent*>(component)) {
		auralizingComponents.push_back(auralComp);
		for (const auto& indirectSend : auralComp->indirectSends) {
			indirectSend->receiver->indirectSends.push_back(indirectSend);
		}
	}

	components.push_back(component);
}

void AudioScene::disconnectAudioComponent(AudioComponent* component)
{
	// direct connections
	for (const auto& output : component->outputs) {
		output->dest->inputs.remove(output);
	}

	for (const auto& input : component->inputs) {
		input->source->outputs.remove(input);
		if (auto* gComp = dynamic_cast<GeneratingAudioComponent*>(input->source)) {
			gComp->removeConsumer(input->genID);
		}
	}

	// indirect connections
	if (auto* outComp = dynamic_cast<OutputAudioComponent*>(component)) {
		outputComponents.remove(outComp);
		for (const auto& indirectSend : outComp->indirectSends) {
			indirectSend->sender->indirectSends.remove(indirectSend);
		}
	}

	if (auto* auralComp = dynamic_cast<AuralizingAudioComponent*>(component)) {
		auralizingComponents.remove(auralComp);
		for (const auto& indirectSend : auralComp->indirectSends) {
			indirectSend->receiver->indirectSends.remove(indirectSend);
		}
	}

	components.remove(component);
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
	auto* genComp = dynamic_cast<GeneratingAudioComponent*>(component.get());
	auto* auralComp = dynamic_cast<AuralizingAudioComponent*>(component.get());
	auto* outComp = dynamic_cast<OutputAudioComponent*>(component.get());

	for (AudioComponent* otherComp : components) {
		// outputs
		if (component->bAcceptsOutput && otherComp->bAcceptsInput) {
			// direct send
			auto output = std::make_shared<ADelayLine>(component.get(), otherComp);
			if (genComp) output->genID = genComp->addConsumer();
			component->outputs.push_back(output);
			
			// indirect send
			if (auto* otherOutComp = dynamic_cast<OutputAudioComponent*>(otherComp)) {
				if (auralComp) {
					auto&& indirectSend = std::make_shared<IndirectSend>(auralComp, otherOutComp);
					auralComp->indirectSends.push_back(indirectSend);
				}
			}
		}

		// inputs
		if (component->bAcceptsInput && otherComp->bAcceptsOutput) {
			// direct receive
			auto input = std::make_shared<ADelayLine>(otherComp, component.get());
			component->inputs.push_back(input);

			// indirect receive
			if (auto* otherAuralComp = dynamic_cast<AuralizingAudioComponent*>(otherComp)) {
				if (outComp) {
					auto&& indirectSend = std::make_shared<IndirectSend>(otherAuralComp, outComp);
					outComp->indirectSends.push_back(indirectSend);
				}
			}
		}
	}

	// attach component to AudioObject and pass to AudioEngine
	object->audioComponent = component.get();
	audioEngine->registerComponent(std::move(component), this);
	return object->audioComponent;
}
