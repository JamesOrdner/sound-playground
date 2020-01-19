#pragma once

#include "../SystemObjectInterface.h"
#include "../../Util/Observer.h"
#include "../../Util/Matrix.h"
#include <string>

class PhysicsObject : public SystemObjectInterface, public ObserverInterface
{
public:

	PhysicsObject(const class SystemSceneInterface* scene, const class UObject* uobject);

	virtual ~PhysicsObject();

	void setPhysicsMesh(std::string filepath);

	// Returns the world transform matrix of this component
	const mat::mat4& transformMatrix();

	float raycast(const mat::vec3& origin, const mat::vec3& direction, mat::vec3& hit);

private:

	// World space local location
	mat::vec3 position, parentPosition;

	// World space local rotation
	mat::vec3 rotation, parentRotation;

	// World space local scale
	mat::vec3 scale, parentScale;

	// Global model transform, cumulates parent transforms.
	// Note: This should never be accessed directly! It is updated lazily and may be out of date.
	// Use transformMatrix() instead.
	mat::mat4 transform;

	bool bDirtyTransform;

	// Pointer to shared physics mesh
	class PhysicsMesh* mesh;
};
