#include "AudioEngine.h"
#include "ADelayLine.h"
#include "AudioComponent.h"
#include "AudioOutputComponent.h"
#include "../Engine/EObject.h"
#include <SDL_audio.h>
#include <stdio.h>

void sdl_process_float(void* data, Uint8* stream, int length) {
	length /= sizeof(float) / sizeof(Uint8);
	((AudioEngine*) data)->process_float((float*) stream, length);
}

AudioEngine::AudioEngine() :
	deviceID(0), 
	sampleRate(0.f), 
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
		sampleRate = static_cast<float>(obtainedSpec.freq);
		channels = obtainedSpec.channels;
		bufferLength = obtainedSpec.samples;
		for (const auto& component : components) {
			component->init(sampleRate, bufferLength, channels);
		}
		return true;
	}
}

void AudioEngine::deinit()
{
	if (deviceID < 2) return;
	SDL_CloseAudioDevice(deviceID);
	for (const auto& c : components) c->deinit();
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
	size_t n = length / channels;

	// preprocess
	for (const auto& c : components) c->preprocess();

	// process
	// naive just-get-it-working ordering
	std::vector<size_t> remaining(components.size(), n);
	bool done = false;
	while (!done) {
		size_t i = 0;
		done = true;
		for (const auto& c : components) {
			// update transforms
			if (c->bDirtyTransform) {
				auto ptr = c->m_owner.lock();
				c->m_position = ptr->position();
				c->m_forward = ptr->forward();
				c->bDirtyTransform = false;
				for (auto& op : c->outputs) op->updateDelay(sampleRate, 250);
				for (auto& ip : c->inputs)  ip->updateDelay(sampleRate, 250);
			}

			// process
			remaining[i] -= c->process(remaining[i]);
			if (remaining[i++] && std::dynamic_pointer_cast<AudioOutputComponent>(c)) done = false;
			i %= remaining.size();
		}
	}

	// output
	for (int i = 0; i < length; i++) buffer[i] = 0.f;
	for (const auto& c : components) {
		if (const auto& outputComponent = std::dynamic_pointer_cast<AudioOutputComponent>(c)) {
			const auto& cOut = outputComponent->collectOutput();
			for (int i = 0; i < length; i++) buffer[i] += cOut[i];
		}
	}
}

void AudioEngine::registerComponent(
	const std::shared_ptr<AudioComponent>& component,
	const std::shared_ptr<EObject>& owner)
{
	component->m_owner = owner;
	component->m_position = owner->position();
	component->m_forward = owner->forward();

	if (deviceID >= 2) component->init(sampleRate, channels, bufferLength);

	// Setup delay lines
	const mat::vec3& thisPos = component->position();
	for (const auto& compOther : components) {
		// outputs
		if (component->bAcceptsOutput && compOther->bAcceptsInput) {
			auto output = std::make_shared<ADelayLine>(component, compOther);
			output->init(sampleRate);
			component->outputs.push_back(output);
			compOther->inputs.push_back(output);
		}

		// inputs
		if (component->bAcceptsInput && compOther->bAcceptsOutput) {
			auto input = std::make_shared<ADelayLine>(compOther, component);
			input->init(sampleRate);
			compOther->outputs.push_back(input);
			component->inputs.push_back(input);
		}
	}

	// TODO: Sort by dependency for performance
	components.push_back(component);
}

void AudioEngine::unregisterComponent(const std::shared_ptr<AudioComponent>& component)
{
	component->deinit();
	components.remove(component);

	// Remove delay lines
	for (const auto& input : component->inputs) {
		input->source.lock()->outputs.remove(input);
	}

	for (const auto& output : component->outputs) {
		output->dest.lock()->inputs.remove(output);
	}
}
