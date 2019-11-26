#include "ADelayLine.h"
#include <algorithm>

ReadWriteBuffer::ReadWriteBuffer(size_t samples) :
	buffer(samples),
	readPtr(0), 
	writePtr(0),
	available(samples)
{
}

void ReadWriteBuffer::init(size_t samples)
{
	buffer.resize(samples);
	readPtr = 0;
	writePtr = 0;
	available = samples;
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

ADelayLine::ADelayLine(size_t samples) :
	buffer(samples)
{
}
