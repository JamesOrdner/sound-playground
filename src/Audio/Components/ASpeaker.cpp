#include "ASpeaker.h"
#include "../ADelayLine.h"
#include "../../Engine/EObject.h"
#include <SDL.h>
#include <fftw3.h>

ASpeaker::ASpeaker() :
	inputSignalPtr(0)
{
	bAcceptsInput = false;
	bAcceptsOutput = true;

	SDL_AudioSpec s;
	Uint8* data;
	Uint32 n;
	if (!SDL_LoadWAV("res/sound/auratone_8192.wav", &s, &data, &n)) {
		fprintf(stderr, "Could not open wav: %s\n", SDL_GetError());
	}
	else {
		SDL_assert(s.format == AUDIO_S16);
		inputSignalBuffer.resize(s.samples);
		ir.reserve(s.samples);
		Sint16* sData = (Sint16*) data;
		for (Uint16 i = 0; i < s.samples; i++) {
			ir.push_back(static_cast<float>(sData[i]) / SDL_MAX_SINT16);
		}
		SDL_FreeWAV(data);
	}
}

size_t ASpeaker::process(size_t n)
{
	size_t p = pushCount();
	if (p < n) n = p;
	size_t size = ir.size();
	for (const auto& output : outputs) {
		//float gain = calcGain(output->dest.lock()->position());
		for (size_t i = 0; i < n; i++) {
			float s = static_cast<float>(rand()) / RAND_MAX * 2.f - 1.f;
			/*inputSignalBuffer[inputSignalPtr] = s;
			float out = 0.f;
			for (size_t k = 0; k <= inputSignalPtr; k++) {
				out += inputSignalBuffer[inputSignalPtr - k] * ir[k];
			}
			for (size_t k = inputSignalPtr + 1; k < size; k++) {
				out += inputSignalBuffer[size + inputSignalPtr - k] * ir[k];
			}*/
			output->buffer.push(s);
			inputSignalPtr = (inputSignalPtr + 1) % size;
		}
	}
	return n;
}

float ASpeaker::calcGain(const mat::vec3& dest)
{
	using namespace mat;
	vec3 dir = normal(dest - position());
	const vec3& forward = owner.lock()->forward();
	return dot(dir, forward) * 0.5f + 0.5f;
}
