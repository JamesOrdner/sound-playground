#include "ADelayLine.h"
#include <algorithm>

ReadWriteBuffer::ReadWriteBuffer(size_t sampleDelay) :
	buffer(sampleDelay),
	readPtr(0), 
	writePtr(0),
	available(sampleDelay)
{
}

void ReadWriteBuffer::init(size_t sampleDelay)
{
	buffer.resize(sampleDelay);
	readPtr = 0;
	writePtr = 0;
	available = sampleDelay;
}

size_t ReadWriteBuffer::push(float sample)
{
	if (available == buffer.size()) return 0;
	buffer[writePtr++] = sample;
	if (writePtr == buffer.size()) writePtr = 0;
	available++;
	return 1;
}

size_t ReadWriteBuffer::push(float* samples, size_t n)
{
	size_t f = buffer.size() - available; // free space
	if (n > f) n = f;
	if (writePtr + n > buffer.size()) {
		size_t nEnd = buffer.size() - writePtr;
		size_t nFront = n - nEnd;
		std::copy_n(samples, nEnd, &buffer[writePtr]);
		std::copy_n(samples + nEnd, nFront, &buffer[0]);
	}
	else {
		std::copy_n(samples, n, &buffer[writePtr]);
	}
	writePtr = (writePtr + n) % buffer.size();
	available += n;
	return n;
}

size_t ReadWriteBuffer::read(float* samples, size_t n)
{
	if (n > available) n = available;
	if (readPtr + n > buffer.size()) {
		size_t nEnd = buffer.size() - readPtr;
		size_t nFront = n - nEnd;
		std::copy_n(&buffer[readPtr], nEnd, samples);
		std::copy_n(&buffer[0], nFront, samples + nEnd);
	}
	else {
		std::copy_n(&buffer[readPtr], n, samples);
	}
	readPtr = (readPtr + n) % buffer.size();
	available -= n;
	return n;
}

void ReadWriteBuffer::resize(size_t sampleDelay, size_t interpLastN)
{
	size_t interpIdxLow, interpIdxHigh; // interp pointers within this range
	interpIdxLow = rsub(writePtr, interpLastN);
	interpIdxHigh = writePtr;

	size_t& ptr = readPtr;
	if (interpIdxLow < ptr && ptr < interpIdxHigh) {
		ptr += interpLastN / rdiff(ptr, interpIdxLow);
	}
	else if (ptr >= interpIdxHigh) {
		ptr += interpIdxHigh;
	}
	
	buffer.resize(sampleDelay);
}

size_t ReadWriteBuffer::size()
{
	return buffer.size();
}

size_t ReadWriteBuffer::pushCount()
{
	return buffer.size() - available;
}

size_t ReadWriteBuffer::pullCount()
{
	return available;
}

void ReadWriteBuffer::resample(size_t rangeStartIdx, size_t oldLength, size_t newLength)
{
	double ratio = static_cast<double>(oldLength) / static_cast<double>(newLength);
	if (oldLength >= newLength) {
		resample_forward(rangeStartIdx, newLength, ratio);
	}
	else {
		resample_back(radd(rangeStartIdx, oldLength), newLength, ratio);
	}
}

void ReadWriteBuffer::resample_forward(size_t rangeStartIdx, size_t newLength, double ratio) {
	size_t startIdx = rangeStartIdx;
	float b[4] = { // read ahead buffer, necessary when in-place resampling overlaps
		buffer[rsub(startIdx, size_t(1))],
		buffer[     startIdx            ],
		buffer[radd(startIdx, size_t(1))],
		buffer[radd(startIdx, size_t(2))]
	};

	double dStartIdx = static_cast<double>(startIdx);
	size_t lastIdx = startIdx; // move read-ahead buffer when lastIdx differs from iIdx
	for (size_t i = 0; i < newLength; i++) {
		double fIdx = radd(dStartIdx, i * ratio);
		size_t iIdx = static_cast<size_t>(fIdx);

		if (lastIdx != iIdx) {
			lastIdx = iIdx;
			b[0] = b[1];
			b[1] = b[2];
			b[2] = b[3];
			b[3] = buffer[radd(iIdx, size_t(2))];
		}

		buffer[radd(startIdx, i)] = cubic(b, fIdx - static_cast<double>(iIdx));
	}
}

void ReadWriteBuffer::resample_back(size_t rangeStartIdx, size_t newLength, double ratio)
{
	float b[4] = { // read ahead buffer, necessary when in-place resampling overlaps
		buffer[radd(rangeStartIdx, size_t(1))],
		buffer[rangeStartIdx],
		buffer[rsub(rangeStartIdx, size_t(1))],
		buffer[rsub(rangeStartIdx, size_t(2))]
	};

	double dStartIdx = static_cast<double>(rangeStartIdx);
	size_t lastIdx = rangeStartIdx; // move read-ahead buffer when lastIdx differs from iIdx
	for (size_t i = 0; i < newLength; i++) {
		double fIdx = rsub(dStartIdx, i * ratio);
		size_t iIdx = static_cast<size_t>(ceil(fIdx));

		if (lastIdx != iIdx) {
			b[0] = b[1];
			b[1] = b[2];
			b[2] = b[3];
			b[3] = buffer[rsub(iIdx, size_t(2))];
		}

		buffer[rsub(rangeStartIdx, i)] = cubic(b, fIdx - static_cast<double>(iIdx));
	}
}

float ReadWriteBuffer::cubic(float b[], double i)
{
	float t = static_cast<float>(i);
	float a0 = b[3] - b[2] - b[0] + b[1];
	float a1 = b[0] - b[1] - a0;
	float a2 = b[2] - b[0];
	float a3 = b[1];
	return a0 * t * t * t + a1 * t * t + a2 * t + a3;
}

ADelayLine::ADelayLine(size_t samples) :
	buffer(samples)
{
}
