#include "ADelayLine.h"
#include <algorithm>

ReadWriteBuffer::ReadWriteBuffer(size_t size) :
	buffer(size),
	readPtr(0), 
	writePtr(0)
{
}

size_t ReadWriteBuffer::push(float* samples, size_t n)
{
	size_t bufferLen = buffer.size();

	size_t f; // free space
	if (readPtr > writePtr) {
		f = readPtr - writePtr;
	}
	else {
		f = bufferLen - writePtr + readPtr;
	}

	if (n > f) n = f;

	if (writePtr + n > bufferLen) {
		size_t nEnd = bufferLen - writePtr;
		size_t nFront = n - nEnd;
		std::copy_n(samples, nEnd, &buffer[writePtr]);
		std::copy_n(samples + nEnd, nFront, &buffer[0]);
		writePtr = nFront;
	}
	else {
		std::copy_n(samples, n, &buffer[writePtr]);
		writePtr += n;
	}

	return n;
}

size_t ReadWriteBuffer::read(float* samples, size_t n)
{
	size_t bufferLen = buffer.size();

	size_t av; // samples available
	if (writePtr > readPtr) {
		av = writePtr - readPtr;
	}
	else {
		av = bufferLen - readPtr + writePtr;
	}

	if (n > av) n = av;

	if (readPtr + n > bufferLen) {
		size_t nEnd = bufferLen - readPtr;
		size_t nFront = n - nEnd;
		std::copy_n(&buffer[readPtr], nEnd, samples);
		std::copy_n(&buffer[0], nFront, samples + nEnd);
		readPtr = nFront;
	}
	else {
		std::copy_n(&buffer[readPtr], n, samples);
		readPtr += n;
	}

	return n;
}

ADelayLine::ADelayLine(size_t samples) : buffer(samples)
{
}
