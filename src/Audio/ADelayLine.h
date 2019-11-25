#pragma once

#include <vector>
#include <memory>

// Forward declarations
class AudioComponent;

struct ReadWriteBuffer
{
	ReadWriteBuffer(size_t size);

	// Push samples to the buffer. Returns the number of saved samples.
	// This will be less than `n` if the buffer is full.
	size_t push(float* samples, size_t n);

	// Returns the number of samples read. May be less than `n`
	size_t read(float* samples, size_t n);

private:
	std::vector<float> buffer;

	// An index which points to the current read position
	size_t readPtr;

	// An index which points to the current write position
	size_t writePtr;
};

// ADelayLine is used to connect two different, unobstructed AudioComponent objects
struct ADelayLine
{
	// ADelayLine constructed with a delay length of `samples`
	ADelayLine(size_t samples);

	ReadWriteBuffer buffer;

	std::weak_ptr<AudioComponent> source;
	std::weak_ptr<AudioComponent> dest;
};
