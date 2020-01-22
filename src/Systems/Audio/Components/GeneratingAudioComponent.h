#pragma once

#include "AudioComponent.h"
#include <vector>
#include <map>

class GeneratingAudioComponent
{
public:

	GeneratingAudioComponent();

	virtual ~GeneratingAudioComponent();
	
	// Requests that the component generate `count` samples.
	// Returns number of successfully generated samples.
	size_t generate(size_t count);

	// Generate and return a unique consumer ID
	unsigned int addConsumer();

	// Remove a consumer from this generator
	void removeConsumer(unsigned int consumer);

	// Read `n` samples into a consumer's buffer and move the read pointer
	size_t readGenerated(unsigned int consumer, float* buffer, size_t n);

	// Read `n` samples into a consumer's buffer without moving the read pointer
	size_t peekGenerated(unsigned int consumer, float* buffer, size_t n);

	// Move the consumer's read pointer by `n` samples
	size_t seekGenerated(unsigned int consumer, size_t n);

	// The number of samples available for this consumer to read
	size_t readable(unsigned int consumer);

private:

	// This function should fill the provided buffer with `count` generated
	// samples, returning the number of successfully generated samples.
	virtual size_t generateImpl(float* buffer, size_t count) = 0;

	struct ConsumerData
	{
		size_t readPtr;
		bool readable;
	};

	// Maps consumers to their buffer read index
	std::map<unsigned int, ConsumerData> consumers;

	// Stores generated samples for reading
	std::vector<float> genBuffer;

	// Index of the next write index
	size_t writePtr;

	// Number of valid samples in buffer
	size_t size;

	// Simple consumer ID generator, increment each time a consumer is added
	unsigned int nextConsumerID;

	// Iterate through all consumers and set the size of the buffer to the largest
	// number of pending unread samples
	void updateSize();
};
