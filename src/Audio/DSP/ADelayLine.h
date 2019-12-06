#pragma once

#include <vector>
#include <memory>

class ReadWriteBuffer
{
public:

	ReadWriteBuffer();

	// Resize buffer to sampleDelay and allocate enough memory to expand to maxSampleDelay
	void init(size_t sampleDelay, size_t maxSampleDelay);

	// Push a single sample to the buffer. Returns 1 if
	//  successful, or 0 if the buffer is full.
	size_t push(float sample);

	// Push samples to the buffer. Returns the number of saved samples.
	// This will be less than `n` if the buffer is full.
	size_t push(float* samples, size_t n);

	// Returns the number of samples read. May be less than `n`
	size_t read(float* samples, size_t n);

	// Resize the buffer to newLength, interpolating the entire buffer to fit.
	void resize(size_t newLength);

	// Return the active buffer size
	size_t capacity();

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
	size_t m_size;

	// Current capacity of buffer. Will be less than or equal to buffer.size()
	size_t m_capacity;

	// Ring mod addition
	inline size_t radd(size_t lhs, size_t rhs) {
		size_t sum = lhs + rhs;
		return sum >= m_capacity ? sum % m_capacity : sum;
	}

	// Ring mod subtraction
	inline size_t rsub(size_t lhs, size_t rhs) {
		return lhs >= rhs ? lhs - rhs : m_capacity - (rhs - lhs);
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

	// Update the delay length of the delay line. Called after a
	// change in position of either the source or destination.
	void updateDelayLength(float sampleRate);

	// Push samples to the delay line. Returns the number of saved samples, which may be less than `n`
	size_t push(float* samples, size_t n = 1);

	// Return true if this buffer is not full
	bool pushable();

	// Returns the number of samples read, which may be less than `n`
	size_t read(float* samples, size_t n);

	// Returns the number of samples available for reading
	size_t readable();

	// Source audio component
	std::weak_ptr<AudioComponent> source;

	// Destination audio component
	std::weak_ptr<AudioComponent> dest;

private:

	// Stores the four most recent input samples, used for velocity-dependent cubic interpolation
	float b[4];

	// [0, 1), offset from interpBuffer[1]
	float sampleInterpOffset;

	ReadWriteBuffer buffer;
};
