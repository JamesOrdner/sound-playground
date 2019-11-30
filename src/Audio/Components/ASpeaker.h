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

	// Stores input samples to be used as input to fft
	float* inputBuffer;

	// Points to the next write index in inputBuffer
	int inputPtr;

	// Stores output of input fft
	fftwf_complex* inputFFT;

	// Input fft plan
	fftwf_plan inputPlan;

	// Partitioned, complex input frequency-domain delay line
	std::vector<fftwf_complex*> inputFDL;

	// Points to the next write index in inputFDL
	int fdlPtr;

	// Partitioned, complex frequency-domain impulse response slices
	std::vector<fftwf_complex*> ir;

	// Input to the final IFFT after all multiply and add operations
	fftwf_complex* ifftInput;

	// Final IFFT plan
	fftwf_plan ifftPlan;

	// Stores final output samples to be sent to output
	float* outputBuffer;

	// Previous sample, saved for simple directional lpf
	float prev;

	// Calculate the simple gain for a given destination
	float calcGain(const mat::vec3& dest);
};
