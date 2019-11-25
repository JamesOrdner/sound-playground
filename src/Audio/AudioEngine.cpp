#include "AudioEngine.h"
#include "AudioComponent.h"
#include "AudioOutputComponent.h"
#include <SDL_audio.h>
#include <stdio.h>

void sdl_process_float(void* data, Uint8* stream, int length) {
	length /= sizeof(float) / sizeof(Uint8);
	((AudioEngine*) data)->process_float((float*) stream, length);
}

AudioEngine::AudioEngine() :
	deviceID(0), 
	sampleRate(0), 
	channels(0), 
	bufferLength(0)
{
}

bool AudioEngine::init()
{
	SDL_AudioSpec desiredSpec, obtainedSpec;
	desiredSpec.channels = 2;
	desiredSpec.format = AUDIO_F32;
	desiredSpec.samples = 1024;
	desiredSpec.freq = 44100;
	desiredSpec.callback = sdl_process_float;
	desiredSpec.userdata = this;

	deviceID = SDL_OpenAudioDevice(nullptr, false, &desiredSpec, &obtainedSpec, SDL_AUDIO_ALLOW_ANY_CHANGE);
	if (deviceID < 2) {
		printf("Audio initialization error: %s\n", SDL_GetError());
		return false;
	}
	else {
		sampleRate = obtainedSpec.freq;
		channels = obtainedSpec.channels;
		bufferLength = obtainedSpec.samples;
		for (const auto& component : components) {
			component->init(bufferLength, channels);
		}
		return true;
	}
}

void AudioEngine::deinit()
{
	if (deviceID < 2) return;
	SDL_CloseAudioDevice(deviceID);
}

bool AudioEngine::start()
{
	if (deviceID < 2) return false;
	SDL_PauseAudioDevice(deviceID, 0);
	return true;
}

void AudioEngine::stop()
{

}

void AudioEngine::process_float(float* buffer, int length)
{
	// SDL buffer is not pre-filled
	for (int i = 0; i < length; i++) buffer[i] = 0.f;

	size_t n = length / channels;
	for (const auto& component : components) {
		component->process(n);

		// if this component is an output component, collect output and add to buffer
		if (const auto& outputComp = std::dynamic_pointer_cast<AudioOutputComponent>(component)) {
			const auto& b = outputComp->collectOutput();
			for (int i = 0; i < length; i++) buffer[i] += b[i];
		}
	}
}

void AudioEngine::registerComponent(const std::shared_ptr<AudioComponent>& component)
{
	// TODO: Sort by dependency for performance
	components.push_back(component);
	if (deviceID >= 2) component->init(bufferLength, channels);
}

void AudioEngine::unregisterComponent(const std::shared_ptr<AudioComponent>& component)
{
	components.remove(component);
}
