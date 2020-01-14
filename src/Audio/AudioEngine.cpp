#include "AudioEngine.h"
#include "DSP/ADelayLine.h"
#include "Components/AudioComponent.h"
#include "Components/GeneratingAudioComponent.h"
#include "Components/AuralizingAudioComponent.h"
#include "Components/OutputAudioComponent.h"
#include <portaudio.h>
#include <pa_win_wasapi.h>
#include <stdio.h>

int pa_callback(
	const void* input,
	void* output,
	unsigned long frameCount,
	const PaStreamCallbackTimeInfo* timeInfo,
	PaStreamCallbackFlags statusFlags,
	void* userData)
{
	((AudioEngine*)userData)->process_float((float*)output, static_cast<size_t>(frameCount));
	return paContinue;
}

AudioEngine::AudioEngine() :
	audioStream(nullptr),
	sampleRate(48000.f),
	channels(2)
{
}

AudioEngine::~AudioEngine()
{
}

bool AudioEngine::init()
{
	PaError err;
	err = Pa_Initialize();
	if (err != paNoError) {
		printf("PortAudio error: %s\n", Pa_GetErrorText(err));
		return false;
	}

	PaDeviceIndex deviceCount = Pa_GetDeviceCount();
	for (PaDeviceIndex i = 0; i < deviceCount; i++) {
		const PaDeviceInfo* deviceInfo = Pa_GetDeviceInfo(i);
		if (Pa_GetHostApiInfo(deviceInfo->hostApi)->type == paWASAPI && deviceInfo->maxOutputChannels >= 2) {
			sampleRate = static_cast<float>(deviceInfo->defaultSampleRate);

			PaStreamParameters outputParams = {};
			outputParams.device = i;
			outputParams.channelCount = channels;
			outputParams.sampleFormat = paFloat32;
			outputParams.suggestedLatency = deviceInfo->defaultHighOutputLatency;
			err = Pa_OpenStream(
				&audioStream,
				nullptr,
				&outputParams,
				sampleRate,
				paFramesPerBufferUnspecified,
				paNoFlag,
				pa_callback,
				this);

			if (err != paNoError) {
				printf("PortAudio error: %s\n", Pa_GetErrorText(err));
				return false;
			}
			break;
		}
	}

	for (const auto& comp : components) comp->init(sampleRate);
	return true;
}

void AudioEngine::deinit()
{
	if (audioStream) {
		Pa_CloseStream(audioStream);
		audioStream = nullptr;
	}
	Pa_Terminate();
	for (const auto& c : components) c->deinit();
}

bool AudioEngine::start()
{
	if (!audioStream) return false;
	PaError err = Pa_StartStream(audioStream);
	if (err != paNoError) {
		printf("PortAudio error: %s\n", Pa_GetErrorText(err));
		return false;
	}
	else {
		return true;
	}
}

void AudioEngine::stop()
{
	if (!audioStream) return;
	PaError err = Pa_StopStream(audioStream);
	if (err != paNoError) {
		printf("PortAudio error: %s\n", Pa_GetErrorText(err));
	}
}

void AudioEngine::process_float(float* buffer, size_t frames)
{
	// zero output buffer
	std::fill_n(buffer, frames * channels, 0.f);

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

	// TODO: Only notify audio-related observers
	StateManager::instance().notifyObservers();
}

StateManager::ObserverID ids[2]; // TEMP

AudioComponent* AudioEngine::registerComponent(std::unique_ptr<AudioComponent> component, const EObject* owner)
{
	AudioComponent* ptr = component.get();
	ownedComponents.push_back(std::move(component));
	ptr->setOwner(owner);

	auto& stateManager = StateManager::instance();
	ids[0] = stateManager.registerObserver(
		ptr,
		StateManager::EventType::ComponentCreated,
		[this](const StateManager::EventData& data) { registerComponent(data); }
	);

	ids[1] = stateManager.registerObserver(
		ptr,
		StateManager::EventType::ComponentDeleted,
		[this](const StateManager::EventData& data) { unregisterComponent(data); }
	);

	stateManager.event(ptr, StateManager::EventType::ComponentCreated, ptr);

	return ptr;
}

void AudioEngine::registerComponent(const StateManager::EventData& data)
{
	auto* component = std::get<AudioComponent*>(data);
	if (audioStream) component->init(sampleRate);

	// Setup delay lines
	for (AudioComponent* compOther : components) {
		// outputs
		if (component->bAcceptsOutput && compOther->bAcceptsInput) {
			auto output = std::make_shared<ADelayLine>(component, compOther);
			output->init(sampleRate);
			component->outputs.push_back(output);
			compOther->inputs.push_back(output);
			if (auto* gComp = dynamic_cast<GeneratingAudioComponent*>(component)) {
				output->genID = gComp->addConsumer();
			}
		}

		// inputs
		if (component->bAcceptsInput && compOther->bAcceptsOutput) {
			auto input = std::make_shared<ADelayLine>(compOther, component);
			input->init(sampleRate);
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

void AudioEngine::unregisterComponent(const StateManager::EventData& data)
{
	auto* component = std::get<AudioComponent*>(data);

	for (const auto& input : component->inputs) {
		input->source->outputs.remove(input);
	}

	for (const auto& output : component->outputs) {
		output->dest->inputs.remove(output);
		if (auto* gComp = dynamic_cast<GeneratingAudioComponent*>(component)) {
			gComp->removeConsumer(output->genID);
		}
	}

	// Only deinit if engine is running
	if (audioStream) component->deinit();

	components.remove(component);
	outputComponents.remove(dynamic_cast<OutputAudioComponent*>(component));
	auralizingComponents.remove(dynamic_cast<AuralizingAudioComponent*>(component));
	
	ownedComponents.remove_if([component](const auto& data) { return data.get() == component; });

	StateManager::instance().unregisterObserver(ids[0]);
	StateManager::instance().unregisterObserver(ids[1]);
}
