#include "ASpeaker.h"
#include "../ADelayLine.h"

ASpeaker::ASpeaker()
{
	bAcceptsInput = false;
	bAcceptsOutput = true;
}

size_t ASpeaker::process(size_t n)
{
	size_t p = pushCount();
	if (p < n) n = p;
	for (const auto& output : outputs) {
		for (size_t i = 0; i < n; i++) {
			float s = static_cast<float>(rand()) / RAND_MAX * 2.f - 1.f;
			output->buffer.push(s);
		}
	}
	return n;
}
