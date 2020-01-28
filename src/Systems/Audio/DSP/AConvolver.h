#pragma once

#include "ADSPBase.h"
#include <fftw3.h>
#include <string>
#include <vector>
#include <complex>

class AConvolver : public ADSPBase
{
public:

	AConvolver();
	AConvolver(std::string impulseResponseFilepath);

	// Set the impulse response of the convolver. Sample rate is assumed to be that of the session.
	void setIR(const std::vector<float>& newIR);

	// Set the impulse response of the convolver from a filepath
	void setIR(std::string filepath);

	// ADSPBase interface
	void init(float sampleRate) override;
	void deinit() override;
	void process(float* outbuffer, const float* inbuffer = nullptr, size_t n = 1) override;

private:

	typedef std::complex<float> complex;

	// Time-domain impulse response of this convolver. Sample rate is assumed to be that of the session.
	std::vector<float> impulseResponse;

	// Number of FFT partitions, dependent on impulse response length
	size_t partitions;

	// Partitioned, complex frequency-domain impulse response blocks
	std::vector<complex*> impulseResponseFFTs;

	// Stores input samples to be used as input to FFT
	float* inputBuffer;

	// Output of FFT of input stream
	complex* fftResult;

	// Partitioned, complex input frequency-domain delay line
	std::vector<complex*> freqDelayLine;

	// Input to the final IFFT after all multiply and add operations
	complex* ifftInput;

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

	// Called during init(), or if IR is changed after initialization. Assumes valid sampleRate.
	void loadIR();

	// Unload an impulse response and clean up relevant memory. Safe to call multiple times.
	void unloadIR();

	// If condition is true, return true. Otherwise, call deinit() and return false
	bool convassert(bool condition);
};
