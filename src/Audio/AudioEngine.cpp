#include "AudioEngine.h"
#include "DSP/ADelayLine.h"
#include "Components/AudioComponent.h"
#include "Components/GeneratingAudioComponent.h"
#include "Components/OutputAudioComponent.h"
#include "../Engine/EObject.h"
#include <portaudio.h>
#include <pa_win_wasapi.h>
#include <stdio.h>

#include "Components/ASpeaker.h"

int pa_callback(
	const void* input,
	void* output,
	unsigned long frameCount,
	const PaStreamCallbackTimeInfo* timeInfo,
	PaStreamCallbackFlags statusFlags,
	void* userData)
{
	float* out = (float*)output;
	((AudioEngine*)userData)->process_float((float*)output, frameCount);
	return paContinue;
}

AudioEngine::AudioEngine() :
	audioStream(nullptr),
	sampleRate(44100.f),
	channels(2),
	bufferLength(256)
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
		if (Pa_GetHostApiInfo(deviceInfo->hostApi)->type == paWASAPI) {
			PaWasapiStreamInfo wasapiInfo;
			wasapiInfo.size = sizeof(PaWasapiStreamInfo);
			wasapiInfo.hostApiType = paWASAPI;
			wasapiInfo.version = 1;
			wasapiInfo.flags = paWinWasapiExclusive;
			wasapiInfo.channelMask = NULL;
			wasapiInfo.hostProcessorOutput = NULL;
			wasapiInfo.hostProcessorInput = NULL;
			wasapiInfo.threadPriority = eThreadPriorityProAudio;

			PaStreamParameters outputParams;
			outputParams.device = i;
			outputParams.channelCount = channels;
			outputParams.sampleFormat = paFloat32;
			outputParams.suggestedLatency = Pa_GetDeviceInfo(i)->defaultLowOutputLatency;
			outputParams.hostApiSpecificStreamInfo = &wasapiInfo;
			err = Pa_OpenStream(
				&audioStream,
				nullptr,
				&outputParams,
				sampleRate,
				bufferLength,
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

	for (const auto& component : components) {
		component->init(sampleRate, bufferLength, channels);
	}

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

void AudioEngine::process_float(float* buffer, unsigned long frames)
{
	// preprocess
	for (const auto& c : components) c->preprocess();

	// process
	// naive just-get-it-working ordering
	std::vector<size_t> remaining(components.size(), frames);
	bool done = false;
	while (!done) {
		size_t i = 0;
		done = true;
		for (const auto& c : components) {
			// process
			remaining[i] -= c->process(remaining[0]);

			if (std::dynamic_pointer_cast<ASpeaker>(c)) {
				// update transforms
				if (c->bDirtyTransform) {
					auto ptr = c->m_owner.lock();
					c->m_position = ptr->position();
					c->m_forward = ptr->forward();
					c->bDirtyTransform = false;
				}
			}

			if (remaining[i++] && std::dynamic_pointer_cast<OutputAudioComponent>(c)) done = false;
			i %= remaining.size();
		}
	}

	// output
	unsigned long len = frames * channels;
	for (unsigned long i = 0; i < len; i++) buffer[i] = 0.f;
	for (const auto& c : components) {
		if (const auto& outputComponent = std::dynamic_pointer_cast<OutputAudioComponent>(c)) {
			float* cOut = outputComponent->rawOutputBuffer();
			for (unsigned long i = 0; i < len; i++) buffer[i] += cOut[i];
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

	if (audioStream) component->init(sampleRate, channels, bufferLength);

	// Setup delay lines
	const mat::vec3& thisPos = component->position();
	for (const auto& compOther : components) {
		// outputs
		if (component->bAcceptsOutput && compOther->bAcceptsInput) {
			auto output = std::make_shared<ADelayLine>(component, compOther);
			output->init(sampleRate);
			component->outputs.push_back(output);
			compOther->inputs.push_back(output);
			if (const auto& gComp = std::dynamic_pointer_cast<GeneratingAudioComponent>(component)) {
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
	if (const auto& oComp = std::dynamic_pointer_cast<OutputAudioComponent>(component)) {
		for (const auto& compOther : components) {
			if (auto aComp = std::dynamic_pointer_cast<AuralizingAudioComponent>(compOther)) {
				IndirectSend* sendPtr = aComp->registerIndirectReceiver(oComp.get());
				oComp->registerIndirectSend(sendPtr);
			}
		}
	}

	// If AuralizingAudioComponent, setup indirect connections from this object
	if (const auto& aComp = std::dynamic_pointer_cast<AuralizingAudioComponent>(component)) {
		for (const auto& compOther : components) {
			if (auto oComp = std::dynamic_pointer_cast<OutputAudioComponent>(compOther)) {
				IndirectSend* sendPtr = aComp->registerIndirectReceiver(oComp.get());
				oComp->registerIndirectSend(sendPtr);
			}
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
		if (const auto& gComp = std::dynamic_pointer_cast<GeneratingAudioComponent>(component)) {
			gComp->removeConsumer(output->genID);
		}
	}
}
