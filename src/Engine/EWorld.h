#pragma once

#include "../Graphics/Matrix.h"
#include <list>
#include <memory>

// Forward declarations
class EObject;
class EModel;

class EWorld
{
public:

	EWorld();

	// Add an object to the world
	void addObject(const std::shared_ptr<EObject>& object);

	// Remove an object from the world
	void removeObject(const std::shared_ptr<EObject>& object);

	// Perform a raycast. Returns hit model on success.
	EModel* raycast(const mat::vec3& origin, const mat::vec3& direction, mat::vec3& hitLoc);

	// Called once per frame
	void tick(float deltaTime);

private:

	// All objects in this world
	std::list<std::shared_ptr<EObject>> objects;

	// Physics updates are run at most this frequently (seconds)
	float physicsUpdateMinInterval;

	// Time since last physics update
	float physicsDeltaTime;
};
