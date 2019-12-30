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
			// direct
			remaining[i] -= c->process(remaining[0]);
			if (remaining[i++] && dynamic_cast<OutputAudioComponent*>(c)) done = false;
			i %= remaining.size();

			// indirect
			if (auto* aComp = dynamic_cast<AuralizingAudioComponent*>(c)) {
				size_t processed = aComp->indirectFramesProcessed();
				if (processed < frames) {
					aComp->processIndirect(frames - processed);
					done &= aComp->indirectFramesProcessed() == frames;
				}
			}
		}
	}

	// collect output
	unsigned long len = frames * channels;
	for (unsigned long i = 0; i < len; i++) buffer[i] = 0.f;
	for (const auto& c : components) {
		if (auto* outputComponent = dynamic_cast<OutputAudioComponent*>(c)) {
			float* cOut = outputComponent->rawOutputBuffer();
			for (unsigned long i = 0; i < len; i++) buffer[i] += cOut[i] * 0.2f;
		}
	}
}

void AudioEngine::registerComponent(AudioComponent* component, const EObject* owner)
{
	component->owner = owner;
	if (audioStream) component->init(sampleRate, channels, bufferLength);

	// Setup delay lines
	const mat::vec3& thisPos = component->position();
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
		for (AudioComponent* compOther : components) {
			if (auto* aComp = dynamic_cast<AuralizingAudioComponent*>(compOther)) {
				IndirectSend* sendPtr = aComp->registerIndirectReceiver(oComp);
				oComp->registerIndirectSend(sendPtr);
			}
		}
	}

	// If AuralizingAudioComponent, setup indirect connections from this object
	if (auto* aComp = dynamic_cast<AuralizingAudioComponent*>(component)) {
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

void AudioEngine::unregisterComponent(AudioComponent* component)
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

	component->deinit();
	components.remove(component);
}
