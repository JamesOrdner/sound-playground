#pragma once

#include "../Util/Matrix.h"

class ServiceManagerInterface
{
public:

	// PhysicsSystem services

	virtual class UObject* raycast(const class UScene* uscene, const mat::vec3& origin, const mat::vec3& direction, mat::vec3& hit) = 0;
};
