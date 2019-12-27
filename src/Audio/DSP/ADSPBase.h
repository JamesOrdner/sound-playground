#pragma once

class ADSPBase
{
public:

	// Should this DSP object read from `inbuffer` during processing?
	bool bAcceptsInput;

	// Init is called before any processing for a session begins
	virtual void init(float sampleRate) {
		this->sampleRate = sampleRate;
	}

	// Deinit is called after a processing session ends. Free any allocated resources created in init.
	virtual void deinit() {};

	// Process is called during processing and fills `outbuffer` with `n` output samples.
	// If this DSP object accepts input, `inbuffer` contains `n` input samples.
	virtual void process(float* outbuffer, const float* inbuffer, size_t n = 1) = 0;

protected:

	float sampleRate;

	ADSPBase() : bAcceptsInput(false), sampleRate(0) {};
};
