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

	// Current active capcity of buffer. Will be less than buffer.size()
	size_t m_capacity;

	// Resample a contiguous block of memory. Returns the last index written to
	void resample_forward(size_t readStartIdx, size_t writeEndIdx, double ratio);

	// Resample a block of memory starting at the end index. Returns the last index written to
	void resample_back(size_t readStartIdx, size_t writeEndIdx, double ratio);

	// Cubic interpolation 
	float cubic(float buffer[], double index);

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
struct ADelayLine
{
	ADelayLine(const std::weak_ptr<AudioComponent>& source, const std::weak_ptr<AudioComponent>& dest);

	// Init must be called with the session sample rate before use. source and dest
	// pointers must be set before use!
	void init(float sampleRate);

	// Update the delay length of the delay line. Called after a
	// change in position of either the source or destination.
	void updateDelayLength(float sampleRate);

	ReadWriteBuffer buffer;
	std::weak_ptr<AudioComponent> source;
	std::weak_ptr<AudioComponent> dest;
};
