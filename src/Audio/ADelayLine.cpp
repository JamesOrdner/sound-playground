#include "ADelayLine.h"
#include "AudioComponent.h"
#include <algorithm>

// Speed of sound in air (seconds per meter)
constexpr float soundSpeed = 0.0029154518950437f;

// Maximum distance between two objects (meters)
constexpr float maximumDistance = 10.f;

ReadWriteBuffer::ReadWriteBuffer() :
	readPtr(0), 
	writePtr(0),
	m_size(0),
	m_capacity(0)
{
}

void ReadWriteBuffer::init(size_t sampleDelay, size_t maxSampleDelay)
{
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
	writePtr = (writePtr + n) % m_capacity;
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
	readPtr = (readPtr + n) % m_capacity;
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

void ReadWriteBuffer::resize(size_t newLength, size_t interpLastN)
{
	if (newLength == m_capacity) return;
	
	if (m_size == 0) {
		// buffer empty, reset and return
		readPtr = 0;
		writePtr = 0;
		m_capacity = newLength;
		return;
	}

	size_t capacityDiff = rdiff(newLength, m_capacity);
	double ratio = static_cast<double>(m_capacity) / static_cast<double>(newLength);

	// determine interp indices
	if (interpLastN > m_capacity) interpLastN = m_capacity;
	size_t interpStartIdx = rsub(writePtr, interpLastN);
	size_t interpEndIdx = rsub(writePtr, size_t(1));

	// length of both interpolation sections (first index is 0 if full interp range is contiguous).
	// interpWriteLengths[0] begins at physical buffer location 0 and runs to interpEndIdx.
	// interpWriteLengths[1] begins at interpStartIndex and runs to the end of the buffer.
	size_t interpWriteLengths[2];
	if (newLength < m_capacity) {
		size_t resizedInterpRange = interpLastN - capacityDiff;
		if (interpEndIdx < interpStartIdx) {
			interpWriteLengths[0] = interpEndIdx * resizedInterpRange / interpLastN;
			interpWriteLengths[1] = resizedInterpRange - interpWriteLengths[0];
		}
		else {
			interpWriteLengths[0] = 0;
			interpWriteLengths[1] = resizedInterpRange;
		}
	}
	else {

	}

	if (newLength < m_capacity) { // shrink buffer, forward resample
		resample_forward(0, interpWriteLengths[0], ratio);
		resample_forward(interpStartIdx, interpWriteLengths[1], ratio);
		if (writePtr != 0) {
			std::copy(
				buffer.begin() + writePtr,
				buffer.begin() + m_capacity,
				buffer.begin() + (interpWriteLengths[0] ? interpWriteLengths[0] : writePtr - capacityDiff)
			);
		}

		m_capacity = newLength;
		writePtr = interpWriteLengths[0] ? interpEndIdx : rsub(writePtr, capacityDiff);
		if (interpStartIdx <= readPtr && readPtr <= interpEndIdx ||
			interpWriteLengths[0] && (readPtr <= interpStartIdx || interpEndIdx <= readPtr)) {
			// offset from beginning of interp region
			size_t readPtrOffset = readPtr > interpStartIdx ? readPtr - interpStartIdx : readPtr;
			size_t newReadPtrOffset = static_cast<size_t>(readPtrOffset * (1 / ratio));
			size_t reduction = readPtrOffset - newReadPtrOffset;
			readPtr = rsub(readPtr, reduction);
			m_size = reduction < m_size ? m_size - reduction : 0;
		}
		else if (interpEndIdx < readPtr) {
			// readPtr in non-interp region but needs to be moved back
			size_t reduction = interpWriteLengths[0] ? interpEndIdx - interpWriteLengths[0] : capacityDiff;
			readPtr = rsub(readPtr, reduction);
			m_size = reduction < m_size ? m_size - reduction : 0;
		}
	}
	else { // expand buffer, backwards resample

	}
}

void ReadWriteBuffer::resample_forward(
	size_t startIdx,
	size_t writeLength,
	double ratio)
{
	float b[4] = { // read ahead buffer, necessary when in-place resampling overlaps
		buffer[rsub(startIdx, size_t(1))],
		buffer[startIdx],
		buffer[radd(startIdx, size_t(1))],
		buffer[radd(startIdx, size_t(2))]
	};

	double dStartIdx = static_cast<double>(startIdx);
	size_t lastIdx = startIdx; // move read-ahead buffer when lastIdx differs from iIdx
	for (size_t i = 0; i < writeLength; i++) {
		double fIdx = radd(dStartIdx, i * ratio);
		size_t iIdx = static_cast<size_t>(fIdx);

		while (lastIdx < iIdx) {
			lastIdx++;
			b[0] = b[1];
			b[1] = b[2];
			b[2] = b[3];
			b[3] = buffer[radd(lastIdx, size_t(2))];
		}

		buffer[startIdx + i] = cubic(b, fIdx - static_cast<double>(iIdx));
	}
}

//void ReadWriteBuffer::resample_forward(
//	size_t startIdx,
//	size_t writeLength,
//	size_t ringSize,
//	double ratio)
//{
//	float b[4] = { // read ahead buffer, necessary when in-place resampling overlaps
//		buffer[rsub(startIdx, size_t(1), ringSize)],
//		buffer[startIdx],
//		buffer[radd(startIdx, size_t(1), ringSize)],
//		buffer[radd(startIdx, size_t(2), ringSize)]
//	};
//
//	double dStartIdx = static_cast<double>(startIdx);
//	size_t lastIdx = startIdx; // move read-ahead buffer when lastIdx differs from iIdx
//	for (size_t i = 0; i < writeLength; i++) {
//		double fIdx = radd(dStartIdx, i * ratio, ringSize);
//		size_t iIdx = static_cast<size_t>(fIdx);
//
//		while (lastIdx < iIdx) {
//			lastIdx++;
//			b[0] = b[1];
//			b[1] = b[2];
//			b[2] = b[3];
//			b[3] = buffer[radd(lastIdx, size_t(2), ringSize)];
//		}
//
//		buffer[startIdx + i] = cubic(b, fIdx - static_cast<double>(iIdx));
//	}
//}

void ReadWriteBuffer::resample_back(
	size_t readStartIdx,
	size_t writeStartIdx, 
	size_t writeLength, 
	size_t ringSize, 
	double ratio)
{
	float b[4] = { // read ahead buffer, necessary when in-place resampling overlaps
		buffer[radd(readStartIdx, size_t(1), ringSize)],
		buffer[readStartIdx],
		buffer[rsub(readStartIdx, size_t(1), ringSize)],
		buffer[rsub(readStartIdx, size_t(2), ringSize)]
	};

	double dStartIdx = static_cast<double>(readStartIdx);
	size_t lastIdx = readStartIdx; // move read-ahead buffer when lastIdx differs from iIdx
	for (size_t i = 0; i < writeLength; i++) {
		double fIdx = rsub(dStartIdx, i * ratio, ringSize);
		size_t iIdx = static_cast<size_t>(ceil(fIdx));

		if (lastIdx != iIdx) {
			b[0] = b[1];
			b[1] = b[2];
			b[2] = b[3];
			b[3] = buffer[rsub(iIdx, size_t(2), ringSize)];
		}

		buffer[writeStartIdx - i] = cubic(b, fIdx - static_cast<double>(iIdx));
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

void ADelayLine::updateDelay(float sampleRate, size_t interpSamples)
{
	float dist = mat::dist(source.lock()->position(), dest.lock()->position());
	size_t sampleDelay = static_cast<size_t>(sampleRate * dist * soundSpeed);
	buffer.resize(sampleDelay, interpSamples);
}
