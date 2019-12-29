#include "GeneratingAudioComponent.h"

// Hardcoded buffer capacity, may want to change this in the future
constexpr size_t capacity = 512;

GeneratingAudioComponent::GeneratingAudioComponent() :
	genBuffer(capacity),
	writePtr(0),
	size(0)
{
}

size_t GeneratingAudioComponent::generate(size_t count)
{
	if (capacity == size) return 0;
	size_t f = capacity - size; // free space
	if (count > f) count = f;

	size_t genCount;
	if (writePtr + count > capacity) {
		size_t nEnd = capacity - writePtr;
		genCount = generate(genBuffer.data() + writePtr, nEnd);
		if (genCount < nEnd) {
			writePtr += genCount;
		}
		else {
			writePtr = generate(genBuffer.data(), count - nEnd);
			genCount += writePtr;
		}
	}
	else {
		genCount = generate(genBuffer.data() + writePtr, count);
		writePtr += genCount;
		if (writePtr == capacity) writePtr = 0;
	}

	// If we added samples to the buffer, all consumers may now read
	if (genCount > 0) {
		for (auto& c : consumers) c.second.readable = true;
	}

	return genCount;
}

void GeneratingAudioComponent::addConsumer(void* consumer)
{
	ConsumerData cData;
	cData.readPtr = writePtr;
	cData.readable = false;
	consumers[consumer] = cData;
}

void GeneratingAudioComponent::removeConsumer(void* consumer)
{
	consumers.erase(consumer);
}

size_t GeneratingAudioComponent::readGenerated(void* consumer, float* buffer, size_t n)
{
	size_t readCount = peekGenerated(consumer, buffer, n);
	return seekGenerated(consumer, readCount);
}

size_t GeneratingAudioComponent::peekGenerated(void* consumer, float* buffer, size_t n)
{
	size_t readCount = readable(consumer);
	if (readCount == 0) return 0;
	if (n > readCount) n = readCount;

	size_t readPtr = consumers.at(consumer).readPtr;
	if (readPtr + n > capacity) {
		size_t nEnd = capacity - readPtr;
		size_t nFront = n - nEnd;
		std::copy_n(genBuffer.data() + readPtr, nEnd, buffer);
		std::copy_n(genBuffer.data(), nFront, buffer + nEnd);
	}
	else {
		std::copy_n(genBuffer.data() + readPtr, n, buffer);
	}

	return n;
}

size_t GeneratingAudioComponent::seekGenerated(void* consumer, size_t n)
{
	size_t readCount = readable(consumer);
	if (readCount == 0) return 0;
	if (n > readCount) n = readCount;

	ConsumerData& cData = consumers.at(consumer);
	cData.readPtr += n;
	if (cData.readPtr >= capacity) cData.readPtr -= capacity;
	if (cData.readPtr = writePtr) cData.readable = false;
	
	// If this consumer had the largest pending sample count, we
	// may be able to update (i.e. decrease) the overall buffer size
	if (readCount == size) updateSize();

	return n;
}

size_t GeneratingAudioComponent::readable(void* consumer)
{
	ConsumerData& cData = consumers.at(consumer);
	if (!cData.readable) return 0;
	if (writePtr > cData.readPtr) {
		return writePtr - cData.readPtr;
	}
	else {
		return capacity - cData.readPtr + writePtr;
	}
}

void GeneratingAudioComponent::updateSize()
{
	size = 0;
	for (auto& c : consumers) {
		size_t cSize = readable(c.first);
		if (cSize > size) size = cSize;
	}
}
