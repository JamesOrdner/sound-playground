#pragma once

struct AInterpParameter
{
	// Target parameter value
	float target;

	// Current parameter value
	float current;

	// Interp rate time constant (seconds)
	float rate;

	// Sample rate, must set initialized before use
	float sampleRate;

	AInterpParameter(float initialValue, float interpRate = 0.1f);

	// Update the parameter by the provided number of steps.
	// If no parameter is given, value is updated by a single step.
	// Returns the new current value.
	float update(size_t steps = 1);
};
