#include "ASpeaker.h"
#include "../ADelayLine.h"
#include "../../Engine/EObject.h"

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
		float gain = calcGain(output->dest.lock()->getPosition());
		for (size_t i = 0; i < n; i++) {
			float s = static_cast<float>(rand()) / RAND_MAX * 2.f - 1.f;
			output->buffer.push(s * gain);
		}
	}
	return n;
}

float ASpeaker::calcGain(const mat::vec3& dest)
{
	using namespace mat;
	vec3 dir = normal(dest - getPosition());
	const vec3& forward = owner.lock()->forward();
	return dot(dir, forward) * 0.5f + 0.5f;
}
