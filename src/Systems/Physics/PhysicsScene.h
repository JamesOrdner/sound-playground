#pragma once

#include "../SystemSceneInterface.h"
#include "../../Util/Matrix.h"
#include <list>
#include <memory>

class PhysicsScene : public SystemSceneInterface
{
public:

	PhysicsScene(const class SystemInterface* system, const class UScene* uscene);

	~PhysicsScene();

	const class UObject* raycast(const mat::vec3& origin, const mat::vec3& direction, mat::vec3& hit) const;

private:

	std::list<std::unique_ptr<class PhysicsObject>> physicsObjects;

	SystemObjectInterface* addSystemObject(SystemObjectInterface* object) override;
};
