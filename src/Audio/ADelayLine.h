#pragma once

#include <vector>
#include <memory>

class ReadWriteBuffer
{
public:

	// If sampleDelay is known during construction, pass it here and init()
	// may be skipped. Otherwise pass 0 and call init() later.
	ReadWriteBuffer(size_t samples);

	// Resize buffer to sampleDelay
	void init(size_t samples);

	// Push a single sample to the buffer. Returns 1 if
	//  successful, or 0 if the buffer is full.
	size_t push(float sample);

	// Push samples to the buffer. Returns the number of saved samples.
	// This will be less than `n` if the buffer is full.
	size_t push(float* samples, size_t n);

	// Returns the number of samples read. May be less than `n`
	size_t read(float* samples, size_t n);

	// Returns the sample length of the buffer
	size_t size();

	// Return the number of samples that can be pushed
	size_t pushCount();

	// Return the number of samples that can be pulled
	size_t pullCount();

private:
	std::vector<float> buffer;

	// An index which points to the current read position
	size_t readPtr;

	// An index which points to the current write position
	size_t writePtr;

	// Number of samples available for reading
	size_t available;
};

// Forward declarations
class AudioComponent;

// ADelayLine is used to connect two different, unobstructed AudioComponent objects
struct ADelayLine
{
	// ADelayLine constructed with a delay length of `samples`
	ADelayLine(size_t samples);

	ReadWriteBuffer buffer;

	std::weak_ptr<AudioComponent> source;
	std::weak_ptr<AudioComponent> dest;
};
