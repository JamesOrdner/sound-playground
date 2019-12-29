#pragma once

#include "AudioComponent.h"
#include <vector>
#include <map>

class GeneratingAudioComponent : public AudioComponent
{
public:

	GeneratingAudioComponent();

	// Requests that the component generate `count` samples.
	// Returns number of successfully generated samples.
	size_t generate(size_t count);

protected:

	void addConsumer(void* consumer);
	void removeConsumer(void* consumer);

	size_t readGenerated(void* consumer, float* buffer, size_t n);
	size_t peekGenerated(void* consumer, float* buffer, size_t n);
	size_t seekGenerated(void* consumer, size_t n);
	size_t readable(void* consumer);

private:

	// This function will fill the provided buffer with `count` generated
	// samples, and return the number of successfully generated samples.
	virtual size_t generate(float* buffer, size_t count) = 0;

	// Stores generated samples for reading
	std::vector<float> genBuffer;

	// Index of the next write index
	size_t writePtr;

	// Number of valid samples in buffer
	size_t size;

	struct ConsumerData
	{
		size_t readPtr;
		bool readable;
	};

	// Maps consumers to their buffer read index
	std::map<void*, ConsumerData> consumers;

	// Iterate through all consumers and set the size of the buffer to the largest
	// number of pending unread samples
	void updateSize();
};
