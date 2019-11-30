#pragma once

#include "../AudioComponent.h"
#include <fftw3.h>

class ASpeaker : public AudioComponent
{
public:

	ASpeaker();

	~ASpeaker();

	// AudioComponent interface
	size_t process(size_t n) override;

private:

	// Number of FFT partitions, dependent on impulse response length
	size_t partitions;

	// Partitioned, complex frequency-domain impulse response blocks
	std::vector<fftwf_complex*> ir;

	// Stores input samples to be used as input to FFT
	float* inputBuffer;

	// Points to the next write index in inputBuffer
	int inputPtr;

	// Stores output of input FFT
	fftwf_complex* fftOutput;

	// Input FFT plan
	fftwf_plan fftPlan;

	// Partitioned, complex input frequency-domain delay line
	std::vector<fftwf_complex*> freqDelayLine;

	// Points to the next write index in inputFDL
	int fdlPtr;

	// Input to the final IFFT after all multiply and add operations
	fftwf_complex* ifftInput;

	// Stores final output samples to be sent to output
	float* outputBuffer;

	// Final IFFT plan
	fftwf_plan ifftPlan;

	// Previous sample, saved for simple directional lpf
	float prev;

	// Calculate the simple gain for a given destination
	float calcGain(const mat::vec3& dest);
};
