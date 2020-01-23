#include "AudioEngine.h"
#include "AudioScene.h"
#include "DSP/ADelayLine.h"
#include "Components/AudioComponent.h"
#include "Components/GeneratingAudioComponent.h"
#include "Components/AuralizingAudioComponent.h"
#include "Components/OutputAudioComponent.h"
#include <portaudio.h>
#include <pa_win_wasapi.h>
#include <algorithm>

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

	for (const auto& c : audioComponents) c->init(sampleRate);
	return true;
}

void AudioEngine::deinit()
{
	if (audioStream) {
		Pa_CloseStream(audioStream);
		audioStream = nullptr;
	}
	Pa_Terminate();
	for (const auto& c : audioComponents) c->deinit();
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

	// process audio for all scenes
	for (auto* scene : scenes) scene->processSceneAudio(buffer, frames);

	// handle pending external changes to audio objects
	ExternalAudioEngineEvent event;
	while (externalEventQueue.pop(event)) {
		switch (event.type) {
		case ExternalAudioEngineEvent::Type::ComponentAdded:
			event.scene->connectAudioComponent(event.component);
			break;
		case ExternalAudioEngineEvent::Type::ComponentRemoved:
			event.scene->disconnectAudioComponent(event.component);
			internalEventQueue.push(InternalAudioEngineEvent{ InternalAudioEngineEvent::Type::DeleteComponent, event.component, event.scene });
		}
	}
}

void AudioEngine::tick(float deltaTime)
{
	// handle pending changes created on the audio thread
	InternalAudioEngineEvent event;
	while (internalEventQueue.pop(event)) {
		if (event.type == InternalAudioEngineEvent::Type::DeleteComponent) {
			event.component->deinit();
			for (auto it = audioComponents.cbegin(); it != audioComponents.cend(); it++) {
				if (it->get() == event.component) {
					audioComponents.erase(it);
					break;
				}
			}
			if (!event.scene->registeredComponentCount()) {
				// remove scene if no components remain
				scenes.erase(std::remove(scenes.begin(), scenes.end(), event.scene), scenes.end());
			}
		}
	}
}

void AudioEngine::registerComponent(std::unique_ptr<AudioComponent> component, AudioScene* scene)
{
	component->init(sampleRate);
	externalEventQueue.push(
		ExternalAudioEngineEvent{
			ExternalAudioEngineEvent::Type::ComponentAdded,
			audioComponents.emplace_back(std::move(component)).get(),
			scene
		}
	);

	// add scene if not present in running scenes
	for (const auto* knownScene : scenes) {
		if (knownScene == scene) return;
	}
	scenes.push_back(scene);
}

void AudioEngine::unregisterComponent(AudioComponent* component, AudioScene* scene)
{
	externalEventQueue.push(ExternalAudioEngineEvent{ ExternalAudioEngineEvent::Type::ComponentRemoved, component, scene });
}
