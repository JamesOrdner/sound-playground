#pragma once

#include "../Util/Matrix.h"

class ServiceManagerInterface
{
public:

	// GraphicsSystem services

	// Assigns the screen's pixel dimensions to `x` and `y`
	virtual void screenDimensions(int& x, int& y) const = 0;

	// PhysicsSystem services

	// Raycast objects in a scene, returning the hit object (or nullptr on ray miss)
	virtual const class UObject* raycast(const class UScene* uscene, const mat::vec3& origin, const mat::vec3& direction) const = 0;
	virtual const class UObject* raycast(const class UScene* uscene, const mat::vec3& origin, const mat::vec3& direction, mat::vec3& hit) const = 0;

	// Convenience function allowing screen raycasting from a specified x and y pixel coordinate
	virtual const class UObject* raycastScreen(const class UScene* uscene, int x, int y) const = 0;
	virtual const class UObject* raycastScreen(const class UScene* uscene, int x, int y, mat::vec3& hit) const = 0;
};
