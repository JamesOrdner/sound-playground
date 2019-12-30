#pragma once

#include <vector>
#include <memory>

class ReadWriteBuffer
{
public:

	ReadWriteBuffer();

	// Initialize the buffer with capacity `sampleDelay`. Optionally include `intialSize`,
	// the initial number of zero samples
	void init(size_t delayLength, size_t initialSize = 0);

	// Write a single sample to the buffer. Returns 1 if
	// successful, or 0 if the buffer is full.
	size_t write(float sample);

	// Write samples to the buffer. Returns the number of saved samples.
	// This will be less than `n` if the buffer is full.
	size_t write(float* samples, size_t n);

	// Returns the number of samples read. May be less than `n`
	size_t read(float* samples, size_t n);

	// Return the active buffer size
	size_t capacity();

	// Return the number of samples that can be written
	size_t writeable();

	// Return the number of samples that can be read
	size_t readable();

private:

	std::vector<float> buffer;

	// An index which points to the current read position
	size_t readPtr;

	// An index which points to the current write position
	size_t writePtr;

	// Number of valid samples in the buffer
	size_t size;

	// Ring mod addition
	inline size_t radd(size_t lhs, size_t rhs) {
		size_t sum = lhs + rhs;
		return sum >= buffer.size() ? sum % buffer.size() : sum;
	}

	// Ring mod addition with custom size
	inline size_t radd(size_t lhs, size_t rhs, size_t size) {
		size_t sum = lhs + rhs;
		return sum >= size ? sum % size : sum;
	}

	// Ring mod subtraction
	inline size_t rsub(size_t lhs, size_t rhs) {
		return lhs >= rhs ? lhs - rhs : buffer.size() - (rhs - lhs);
	}

	// Absolute value difference
	inline size_t rdiff(size_t lhs, size_t rhs) {
		return lhs >= rhs ? lhs - rhs : rhs - lhs;
	}
};

// Forward declarations
class AudioComponent;

// ADelayLine is used to connect two different, unobstructed AudioComponent objects
class ADelayLine
{
public:

	ADelayLine(const std::weak_ptr<AudioComponent>& source, const std::weak_ptr<AudioComponent>& dest);

	// Relative velocity of distance between source and destination, in meters per second.
	// Velocity is positive if distance is increasing, or negative if decreasing.
	float velocity;

	// Init must be called with the session sample rate before use
	void init(float sampleRate);

	// Push samples to the delay line. Returns the number of saved samples, which may be less than `n`
	size_t write(float* samples, size_t n = 1);

	// Return true if this buffer is not full
	bool writeable();

	// Returns the number of samples read, which may be less than `n`
	size_t read(float* samples, size_t n);

	// Returns the number of samples available for reading
	size_t readable();

	// Source audio component
	std::weak_ptr<AudioComponent> source;

	// Destination audio component
	std::weak_ptr<AudioComponent> dest;

	// This is a unique ID which associates this delay line with a GeneratingAudioComponent
	unsigned int genID;

private:

	ReadWriteBuffer buffer;

	// Stores the four most recent input samples, used for velocity-dependent cubic interpolation
	float b[4];

	// [0, 1). Fractional sample offset from interpBuffer[1]
	float sampleInterpOffset;
};
