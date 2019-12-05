#include "ADelayLine.h"
#include "AudioComponent.h"
#include <algorithm>

// Speed of sound in air (seconds per meter)
constexpr float soundSpeed = 0.0029154518950437f;

// Maximum distance between two objects (meters)
constexpr float maximumDistance = 10.f;

constexpr size_t minBufferSize = 16;

ReadWriteBuffer::ReadWriteBuffer() :
	readPtr(0), 
	writePtr(0),
	m_size(0),
	m_capacity(0)
{
}

void ReadWriteBuffer::init(size_t sampleDelay, size_t maxSampleDelay)
{
	if (sampleDelay < minBufferSize) sampleDelay = minBufferSize;
	buffer.resize(maxSampleDelay);
	readPtr = 0;
	writePtr = 0;
	m_size = sampleDelay;
	m_capacity = sampleDelay;
}

size_t ReadWriteBuffer::push(float sample)
{
	if (m_size == m_capacity) return 0;
	buffer[writePtr++] = sample;
	if (writePtr == m_capacity) writePtr = 0;
	m_size++;
	return 1;
}

size_t ReadWriteBuffer::push(float* samples, size_t n)
{
	size_t f = m_capacity - m_size; // free space
	if (n > f) n = f;
	if (writePtr + n > m_capacity) {
		size_t nEnd = m_capacity - writePtr;
		size_t nFront = n - nEnd;
		std::copy_n(samples, nEnd, &buffer[writePtr]);
		std::copy_n(samples + nEnd, nFront, &buffer[0]);
	}
	else {
		std::copy_n(samples, n, &buffer[writePtr]);
	}
	writePtr = radd(writePtr, n);
	m_size += n;
	return n;
}

size_t ReadWriteBuffer::read(float* samples, size_t n)
{
	if (n > m_size) n = m_size;
	if (readPtr + n > m_capacity) {
		size_t nEnd = m_capacity - readPtr;
		size_t nFront = n - nEnd;
		std::copy_n(&buffer[readPtr], nEnd, samples);
		std::copy_n(&buffer[0], nFront, samples + nEnd);
	}
	else {
		std::copy_n(&buffer[readPtr], n, samples);
	}
	readPtr = radd(readPtr, n);
	m_size -= n;
	return n;
}

size_t ReadWriteBuffer::capacity()
{
	return m_capacity;
}

size_t ReadWriteBuffer::pushCount()
{
	return m_capacity - m_size;
}

size_t ReadWriteBuffer::pullCount()
{
	return m_size;
}

void ReadWriteBuffer::resize(size_t newLength)
{
	if (newLength < minBufferSize) newLength = minBufferSize;
	if (newLength == m_capacity) return;
	if (m_size == 0) { // buffer empty, reset and return
		readPtr = 0;
		writePtr = 0;
		m_capacity = newLength;
		return;
	}

	float ro = buffer[readPtr];
	float wo = buffer[writePtr];

	double ratio = static_cast<double>(newLength) / static_cast<double>(m_capacity);

	size_t newWritePtr = writePtr * ratio;
	if (ratio < 1.0) { // shrink
		resample_forward(0, newWritePtr ? newWritePtr - 1 : newLength - 1, ratio);
		if (writePtr) resample_forward(writePtr, newLength - 1, ratio);
	}
	else { // expand
		resample_back(m_capacity - 1, newWritePtr, ratio);
		if (writePtr) resample_back(writePtr - 1, 0, ratio);
	}

	readPtr = readPtr * ratio;
	writePtr = newWritePtr;
	m_capacity = newLength;
	if (m_size > 0) {
		m_size = writePtr > readPtr ? writePtr - readPtr : m_capacity - readPtr + writePtr;
	}
}

void ReadWriteBuffer::resample_forward(size_t readStartIdx, size_t writeEndIdx, double ratio)
{
	// End interpolation region at writePtr if true, or at end of buffer if false
	bool bEndAtWritePtr = readStartIdx < writePtr;
	
	float b[4] = { // read ahead buffer, necessary when in-place resampling overlaps
		buffer[readStartIdx == writePtr ? readStartIdx : rsub(readStartIdx, 1)],
		buffer[readStartIdx],
		buffer[radd(readStartIdx, 1)],
		buffer[radd(readStartIdx, 2)]
	};

	double dStartIdx = static_cast<double>(readStartIdx);
	size_t lastIdx = readStartIdx; // move read-ahead buffer when lastIdx differs from iIdx
	size_t writeStartIdx = readStartIdx * ratio;
	for (size_t i = writeStartIdx; i <= writeEndIdx; i++) {
		double fIdx = i / ratio;
		size_t iIdx = static_cast<size_t>(fIdx);

		while (lastIdx < iIdx) {
			lastIdx++;
			b[0] = b[1];
			b[1] = b[2];
			b[2] = b[3];
			size_t indexAhead = radd(lastIdx, 2);
			if ((!bEndAtWritePtr && lastIdx < indexAhead) || (indexAhead < writePtr)) {
				b[3] = buffer[indexAhead];
			}
		}

		buffer[i] = cubic(b, fIdx - static_cast<double>(iIdx));
	}
}

void ReadWriteBuffer::resample_back(size_t readStartIdx, size_t writeEndIdx, double ratio)
{
	// End interpolation region at writePtr if true, or at beginning of buffer if false
	bool bEndAtWritePtr = writePtr < readStartIdx;

	float b[4] = { // read ahead buffer, necessary when in-place resampling overlaps
		buffer[rsub(readStartIdx, 2)],
		buffer[rsub(readStartIdx, 1)],
		buffer[readStartIdx],
		buffer[radd(readStartIdx, 1) == writePtr ? readStartIdx : radd(readStartIdx, 1)]
	};

	double dStartIdx = static_cast<double>(readStartIdx);
	size_t lastIdx = readStartIdx; // move read-ahead buffer when lastIdx differs from iIdx
	size_t i = (readStartIdx + 1) * ratio - 1; // writeStartIdx
	while (true) {
		double fIdx = i / ratio;
		size_t iIdx = static_cast<size_t>(fIdx);

		if (lastIdx > iIdx) {
			lastIdx--;
			b[1] = b[0];
			b[2] = b[1];
			b[3] = b[2];
			size_t indexAhead = rsub(lastIdx, 2);
			if ((!bEndAtWritePtr && lastIdx > indexAhead) || (indexAhead >= writePtr)) {
				b[0] = buffer[indexAhead];
			}
		}

		buffer[i] = cubic(b, 1.0 - (fIdx - static_cast<double>(iIdx)));
		
		if (i-- == writeEndIdx) break;
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

ADelayLine::ADelayLine(const std::weak_ptr<AudioComponent>& source, const std::weak_ptr<AudioComponent>& dest) :
	source(source),
	dest(dest)
{
}

void ADelayLine::init(float sampleRate)
{
	float dist = mat::dist(source.lock()->position(), dest.lock()->position());
	size_t sampleDelay = static_cast<size_t>(sampleRate * dist * soundSpeed);
	size_t maxDelay = static_cast<size_t>(sampleRate * maximumDistance * soundSpeed);
	buffer.init(sampleDelay, maxDelay);
}

void ADelayLine::updateDelayLength(float sampleRate)
{
	float dist = mat::dist(source.lock()->position(), dest.lock()->position());
	size_t sampleDelay = static_cast<size_t>(sampleRate * dist * soundSpeed);
	buffer.resize(sampleDelay);
}
