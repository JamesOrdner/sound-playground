#pragma once

#include "../Util/Matrix.h"

class ServiceManagerInterface
{
public:

	// PhysicsSystem services

	virtual const class UObject* raycast(const class UScene* uscene, const mat::vec3& origin, const mat::vec3& direction, mat::vec3& hit) const = 0;
};
