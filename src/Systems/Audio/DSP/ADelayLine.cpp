#include "ADelayLine.h"
#include "../Components/AudioComponent.h"
#include <algorithm>

// Speed of sound in air (seconds per meter)
constexpr float soundSpeed = 0.0029154518950437f;

// Maximum distance between two objects (meters)
constexpr float maximumDistance = 10.f;

ReadWriteBuffer::ReadWriteBuffer() :
	readPtr(0), 
	writePtr(0),
	size(0)
{
}

void ReadWriteBuffer::init(size_t delayLength, size_t initialSize)
{
	buffer.clear();
	buffer.resize(delayLength);
	readPtr = 0;
	writePtr = initialSize;
	size = initialSize;
}

size_t ReadWriteBuffer::write(float sample)
{
	if (size == buffer.size()) return 0;
	buffer[writePtr++] = sample;
	if (writePtr == buffer.size()) writePtr = 0;
	size++;
	return 1;
}

size_t ReadWriteBuffer::write(float* samples, size_t n)
{
	if (size == buffer.size()) return 0;
	size_t f = buffer.size() - size; // free space
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
	writePtr = radd(writePtr, n);
	size += n;
	return n;
}

size_t ReadWriteBuffer::read(float* samples, size_t n)
{
	if (size == 0) return 0;
	if (n > size) n = size;

	if (readPtr + n > buffer.size()) {
		size_t nEnd = buffer.size() - readPtr;
		size_t nFront = n - nEnd;
		std::copy_n(&buffer[readPtr], nEnd, samples);
		std::copy_n(&buffer[0], nFront, samples + nEnd);
	}
	else {
		std::copy_n(&buffer[readPtr], n, samples);
	}

	size -= n;
	readPtr = radd(readPtr, n);
	return n;
}

size_t ReadWriteBuffer::capacity()
{
	return buffer.size();
}

size_t ReadWriteBuffer::writeable()
{
	return buffer.size() - size;
}

size_t ReadWriteBuffer::readable()
{
	return size;
}

ADelayLine::ADelayLine(AudioComponent* source, AudioComponent* dest) :
	source(source),
	sourceData(nullptr),
	dest(dest),
	destData(nullptr),
	genID(0),
	bInitialized(false),
	b{},
	sampleInterpOffset(0.f)
{
}

float ADelayLine::velocity()
{
	mat::vec3 sourceToDestDir = mat::normal(dest->position - source->position);
	mat::vec3 sourceToDestVel = dest->velocity - source->velocity;
	return mat::dot(sourceToDestDir, sourceToDestVel);
}

void ADelayLine::init(float sampleRate)
{
	if (bInitialized) return;
	bInitialized = true;

	float dist = mat::dist(source->position, dest->position);
	float fMaxSampleDelay = sampleRate * maximumDistance * soundSpeed;
	float fInitSampleDelay = sampleRate * dist * soundSpeed;
	size_t maxSampleDelay = static_cast<size_t>(fMaxSampleDelay);
	size_t initSampleDelay = static_cast<size_t>(fInitSampleDelay);
	buffer.init(maxSampleDelay, initSampleDelay);

	source->initDelayLineData(this, sampleRate, true);
	dest->initDelayLineData(this, sampleRate, false);
}

void ADelayLine::deinit()
{
	if (!bInitialized) return;

	buffer.init(0, 0);
	source->deinitDelayLineData(this, true);
	dest->deinitDelayLineData(this, false);

	bInitialized = false;
}

size_t ADelayLine::write(float* samples, size_t& n)
{
	size_t inputCount = n;
	size_t writeCount = n = 0;
	while (buffer.writeable() && n < inputCount) {
		if (sampleInterpOffset < 1.f) {
			// clamp lower bound, don't allow going back in time
			sampleInterpOffset += std::max(1.f - velocity() * soundSpeed, 0.f);
		}
		
		while (sampleInterpOffset >= 1.f && n < inputCount) {
			b[0] = b[1];
			b[1] = b[2];
			b[2] = b[3];
			b[3] = samples[n++];
			sampleInterpOffset--;
		}

		// out of input samples
		if (sampleInterpOffset >= 1.f) break;
		
		// cubic interp
		float t = sampleInterpOffset;
		float a0 = b[3] - b[2] - b[0] + b[1];
		float a1 = b[0] - b[1] - a0;
		float a2 = b[2] - b[0];
		float a3 = b[1];
		float result = a0 * t * t * t + a1 * t * t + a2 * t + a3;
		writeCount += buffer.write(result);
	}
	return writeCount;

	// DEBUG no doppler
	//size_t inputCount = n;
	//size_t writeCount = n = 0;
	//while (buffer.writeable() && n < inputCount) writeCount += buffer.write(samples[n++]);
	//return writeCount;
}

bool ADelayLine::writeable()
{
	return buffer.writeable();
}

size_t ADelayLine::read(float* samples, size_t n)
{
	return buffer.read(samples, n);
}

size_t ADelayLine::readable()
{
	return buffer.readable();
}
