#pragma once

class ADSPBase
{
public:

	// Should this DSP object read from `inbuffer` during processing?
	bool bAcceptsInput;

	// Has this DSP object been init()ed?
	bool bInitialized;

	// If this is true, the same array may be used for both input and output in process()
	bool bCanProcessInPlace;

	// Init is called before any processing for a session begins
	virtual void init(float sampleRate) {
		this->sampleRate = sampleRate;
		bInitialized = true;
	}

	// Deinit is called after a processing session ends. Free any allocated resources created in init.
	virtual void deinit() {
		bInitialized = false;
	};

	// Process is called during processing and fills `outbuffer` with `n` output samples.
	// If this DSP object accepts input, `inbuffer` contains `n` input samples.
	virtual void process(float* outbuffer, const float* inbuffer, size_t n = 1) = 0;

protected:

	float sampleRate;

	ADSPBase() : 
		bAcceptsInput(false),
		bInitialized(false),
		bCanProcessInPlace(false),
		sampleRate(0) {};
};
