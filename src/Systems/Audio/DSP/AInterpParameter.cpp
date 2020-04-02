#include "AInterpParameter.h"
#include <cmath>

AInterpParameter::AInterpParameter(float initialValue, float interpRate) :
	target(initialValue),
	current(initialValue),
	rate(interpRate),
	sampleRate(44100.f) // best guess
{
}

float AInterpParameter::update(size_t steps)
{
	current += (target - current) * (1.f - expf(-static_cast<float>(steps) / (sampleRate * rate)));
	return current;
}
