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

	float raycast(const mat::vec3& origin, const mat::vec3& direction, mat::vec3& hit) const;

private:

	// World space local location
	mat::vec3 position;

	// World space Euler local rotation
	mat::vec3 rotation;

	// World space local scale
	mat::vec3 scale;

	// Global model transform, cumulates parent transforms
	mat::mat4 transform;

	// Pointer to shared physics mesh
	class PhysicsMesh* mesh;
};
