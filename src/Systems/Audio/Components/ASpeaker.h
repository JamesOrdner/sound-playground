#pragma once

#include "AuralizingAudioComponent.h"
#include <memory>

class ASpeaker : public AuralizingAudioComponent
{
public:

	ASpeaker();

	// AudioComponent interface
	void init(float sampleRate) override;
	void deinit() override;
	void initDelayLineData(class ADelayLine* delayline, float sampleRate, bool bIsSource) override;
	void deinitDelayLineData(class ADelayLine* delayline, bool bIsSource) override;
	size_t process(ADelayLine* output, size_t n) override;

private:

	// Convolves the speaker signal with the speaker IR
	std::unique_ptr<class AConvolver> convolver;

	// Buffer required for process()
	std::vector<float> processingBuffer;

	// GeneratingAudioComponent interface
	size_t generateImpl(float* buffer, size_t count) override;
	size_t sinGeneratorPhase;
};
