#include "AudioEngine.h"
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
	desiredSpec.samples = 1024 * desiredSpec.channels;
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
		bufferLength = obtainedSpec.samples / obtainedSpec.channels;
		SDL_PauseAudioDevice(deviceID, 0);
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
	for (int i = 0; i < length; i++) {
		// silence
		buffer[i] = 0.f;
	}
}
