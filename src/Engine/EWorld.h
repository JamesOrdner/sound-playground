#pragma once

#include "../Graphics/Matrix.h"
#include <list>
#include <unordered_set>
#include <memory>

// Forward declarations
class EObject;
class EModel;
class ECamera;

class EWorld
{
public:

	EWorld();

	// Spawn an object into the world
	template<class T>
	T* spawnObject() {
		return static_cast<T*>(addObject(std::make_unique<T>()));
	};

	// Return a reference to the "global" list of objects
	const std::list<std::unique_ptr<EObject>>& allObjects() const;

	// Returns the active camera
	ECamera* worldCamera() const;

	// Perform a raycast. Returns hit model on success.
	EModel* raycast(
		const mat::vec3& origin,
		const mat::vec3& direction,
		mat::vec3& hitLoc,
		const std::unordered_set<EObject*>& ignore = std::unordered_set<EObject*>()) const;

	// Called once per frame
	void tick(float deltaTime);

private:

	// All objects in this world
	std::list<std::unique_ptr<EObject>> objects;

	// Active camera
	ECamera* camera;

	// Physics updates are run at this frequency (seconds)
	float physicsUpdateInterval;

	// Time since last physics update
	float physicsDeltaTime;

	// Add an object to the world, returning the raw pointer
	EObject* addObject(std::unique_ptr<EObject> object);
};
