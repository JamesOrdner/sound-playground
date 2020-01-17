#pragma once

#include "../Util/Matrix.h"

class PhysicsSystemInterface
{
public:

	virtual class UObject* raycast(const class UScene* uscene, const mat::vec3& origin, const mat::vec3& direction, mat::vec3& hit) = 0;
};