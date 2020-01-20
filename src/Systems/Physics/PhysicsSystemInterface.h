#pragma once

#include "../../Util/Matrix.h"
#include <unordered_set>

class PhysicsSystemInterface
{
public:

	virtual const class UObject* raycast(
		const class UScene* uscene,
		const mat::vec3& origin,
		const mat::vec3& direction,
		mat::vec3& hit,
		const std::unordered_set<const class UObject*>& ignore) const = 0;
};