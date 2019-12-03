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

	// Resize the buffer to newLength, interpolating the last N samples to fit.
	void resize(size_t newLength, size_t interpLastN);

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

	// Resample a contiguous block of memory
	void resample_forward(
		size_t startIdx,
		size_t writeLength,
		double ratio);

	// Resample a block of memory starting at the end index
	void resample_back(
		size_t readStartIdx, 
		size_t writeStartIdx, 
		size_t writeLength,
		size_t ringSize, 
		double ratio);

	// Cubic interpolation 
	float cubic(float buffer[], double index);

	// Ring mod addition
	template<typename T>
	inline T radd(T lhs, T rhs) {
		T max = static_cast<T>(m_capacity);
		T sum = lhs + rhs;
		return sum >= max ? sum - max : sum;
	}

	// Ring mod addition with custom size
	template<typename T>
	inline T radd(T lhs, T rhs, size_t size) {
		T max = static_cast<T>(size);
		T sum = lhs + rhs;
		return sum >= max ? sum - max : sum;
	}

	// Ring mod subtraction
	template<typename T>
	inline T rsub(T lhs, T rhs) {
		T max = static_cast<T>(m_capacity);
		return lhs >= rhs ? lhs - rhs : max - (rhs - lhs);
	}

	// Ring mod subtraction with custom size
	template<typename T>
	inline T rsub(T lhs, T rhs, size_t size) {
		T max = static_cast<T>(size);
		return lhs >= rhs ? lhs - rhs : max - (rhs - lhs);
	}

	// Ring mod difference
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

	// Called after a change in position of either the source or destination. interpSamples
	// is the number of previous samples to stretch to fit the new size, resulting in doppler shifting
	void updateDelay(float sampleRate, size_t interpSamples);

	ReadWriteBuffer buffer;
	std::weak_ptr<AudioComponent> source;
	std::weak_ptr<AudioComponent> dest;
};
