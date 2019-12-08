#pragma once

#include "ADSPBase.h"
#include <fftw3.h>
#include <string>
#include <vector>

class AConvolver : public ADSPBase
{
public:

	AConvolver(std::string impulseResponseFilepath);
	~AConvolver() {};

	// ADSPBase interface
	void init(float sampleRate) override;
	void deinit() override;
	void process(float* outbuffer, float* inbuffer = nullptr, size_t n = 1) override;

private:

	// Filepath of the impulse response file
	std::string filepath;

	// Number of FFT partitions, dependent on impulse response length
	size_t partitions;

	// Partitioned, complex frequency-domain impulse response blocks
	std::vector<fftwf_complex*> impulseResponseFFTs;

	// Stores input samples to be used as input to FFT
	float* inputBuffer;

	// Output of FFT of input stream
	fftwf_complex* fftResult;

	// Partitioned, complex input frequency-domain delay line
	std::vector<fftwf_complex*> freqDelayLine;

	// Input to the final IFFT after all multiply and add operations
	fftwf_complex* ifftInput;

	// Stores final output samples to be sent to output
	float* outputBuffer;

	// Points to the next write index in inputBuffer
	int inputPtr;

	// Points to the next write index in inputFDL
	int fdlPtr;

	// Input FFT plan
	fftwf_plan fftPlan;

	// Output IFFT plan
	fftwf_plan ifftPlan;

	// If condition is true, return true. Otherwise, call deinit() and return false
	inline bool assert(bool condition);
};