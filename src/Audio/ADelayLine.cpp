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
	if (m_size >= m_capacity) return 0;
	buffer[writePtr++] = sample;
	if (writePtr == m_capacity) writePtr = 0;
	m_size++;
	return 1;
}

size_t ReadWriteBuffer::push(float* samples, size_t n)
{
	if (m_size >= m_capacity) return 0;
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
	return;

	if (newLength < minBufferSize) newLength = minBufferSize;
	if (newLength == m_capacity) return;
	if (m_size == 0) { // buffer empty, reset and return
		readPtr = 0;
		writePtr = 0;
		m_capacity = newLength;
		return;
	}

	if (newLength < m_capacity) {
		m_capacity = newLength;
		return;
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
	velocity(0.f),
	b{},
	sampleInterpOffset(0.f),
	source(source),
	dest(dest)
{
}

void ADelayLine::init(float sampleRate)
{
	float dist = mat::dist(source.lock()->position(), dest.lock()->position());
	float fSampleDelay = sampleRate * dist * soundSpeed;
	float fMaxDelay = sampleRate * maximumDistance * soundSpeed;
	size_t sampleDelay = static_cast<size_t>(fSampleDelay);
	size_t maxDelay = static_cast<size_t>(fMaxDelay);
	buffer.init(sampleDelay, maxDelay);
}

void ADelayLine::updateDelayLength(float sampleRate)
{
	float dist = mat::dist(source.lock()->position(), dest.lock()->position());
	float fSampleDelay = sampleRate * dist * soundSpeed;
	size_t sampleDelay = static_cast<size_t>(fSampleDelay);
	buffer.resize(sampleDelay);
}

size_t ADelayLine::push(float* samples, size_t n)
{
	size_t i = 0;
	while (buffer.pushCount() && i < n) {
		if (sampleInterpOffset < 1.f) {
			// clamp lower bound, don't allow going back in time
			sampleInterpOffset += std::max(1.f - velocity * soundSpeed, 0.f);
		}
		
		while (sampleInterpOffset >= 1.f && i < n) {
			b[0] = b[1];
			b[1] = b[2];
			b[2] = b[3];
			b[3] = samples[i++];
			sampleInterpOffset--;
		}

		// out of input samples
		if (sampleInterpOffset >= 1.f) return i;
		
		// cubic interp
		float t = sampleInterpOffset;
		float a0 = b[3] - b[2] - b[0] + b[1];
		float a1 = b[0] - b[1] - a0;
		float a2 = b[2] - b[0];
		float a3 = b[1];
		float result = a0 * t * t * t + a1 * t * t + a2 * t + a3;
		buffer.push(result);
	}
	return i;
}

bool ADelayLine::pushable()
{
	return buffer.pushCount();
}

size_t ADelayLine::read(float* samples, size_t n)
{
	return buffer.read(samples, n);
}

size_t ADelayLine::readable()
{
	return buffer.pullCount();
}
