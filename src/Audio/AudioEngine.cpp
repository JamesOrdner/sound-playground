#include "AudioEngine.h"
#include "ADelayLine.h"
#include "AudioComponent.h"
#include "AudioOutputComponent.h"
#include <SDL_audio.h>
#include <stdio.h>

// Speed of sound in air (seconds per meter)
constexpr float soundSpeed = 0.0029154518950437f;

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
			remaining[i] -= c->process(remaining[i]);
			if (remaining[i++]) done = false;
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
	component->owner = owner;
	if (deviceID >= 2) component->init(bufferLength, channels);

	// Setup delay lines
	const mat::vec3& thisPos = component->position();
	for (const auto& compOther : components) {
		float d = mat::dist(thisPos, compOther->position());
		float t = d * soundSpeed;
		size_t sampleDelay = static_cast<size_t>(t * sampleRate);

		// outputs
		if (component->bAcceptsOutput && compOther->bAcceptsInput) {
			auto output = std::make_shared<ADelayLine>(sampleDelay);
			output->source = component;
			output->dest = compOther;
			component->outputs.push_back(output);
			compOther->inputs.push_back(output);
		}

		// inputs
		if (component->bAcceptsInput && compOther->bAcceptsOutput) {
			auto input = std::make_shared<ADelayLine>(sampleDelay);
			input->source = compOther;
			input->dest = component;
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
