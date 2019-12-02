#pragma once

#include <vector>
#include <memory>

class ReadWriteBuffer
{
public:

	// If sampleDelay is known during construction, pass it here and init()
	// may be skipped. Otherwise pass 0 and call init() later.
	ReadWriteBuffer(size_t sampleDelay);

	// Resize buffer to sampleDelay
	void init(size_t sampleDelay);

	// Push a single sample to the buffer. Returns 1 if
	//  successful, or 0 if the buffer is full.
	size_t push(float sample);

	// Push samples to the buffer. Returns the number of saved samples.
	// This will be less than `n` if the buffer is full.
	size_t push(float* samples, size_t n);

	// Returns the number of samples read. May be less than `n`
	size_t read(float* samples, size_t n);

	// Resize the buffer to new sampleDelay, interpolating the last N samples to fit.
	void resize(size_t sampleDelay, size_t interpLastN);

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

	// Resample audio in-place, beginning at rangeStartIdx, from oldLength samples to newLength samples
	void resample(size_t rangeStartIdx, size_t oldLength, size_t newLength);

	void resample_forward(size_t rangeStartIdx, size_t newLength, double ratio);
	void resample_back(size_t rangeStartIdx, size_t newLength, double ratio);

	// Cubic interpolation 
	float cubic(float buffer[], double index);

	// Ring mod addition
	template<typename T>
	inline T radd(T lhs, T rhs) {
		T max = static_cast<T>(buffer.size());
		T sum = lhs + rhs;
		return sum >= max ? sum - max : sum;
	}

	// Ring mod subtraction
	template<typename T>
	inline T rsub(T lhs, T rhs) {
		T size = static_cast<T>(buffer.size());
		return lhs >= rhs ? lhs - rhs : size - (rhs - lhs);
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
	// ADelayLine constructed with a delay length of `samples`
	ADelayLine(size_t samples);

	ReadWriteBuffer buffer;

	std::weak_ptr<AudioComponent> source;
	std::weak_ptr<AudioComponent> dest;
};
