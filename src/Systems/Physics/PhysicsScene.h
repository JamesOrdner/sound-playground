#pragma once

#include "../SystemSceneInterface.h"
#include "../../Util/Matrix.h"
#include <list>
#include <memory>
#include <unordered_set>

class PhysicsScene : public SystemSceneInterface
{
public:

	PhysicsScene(const class SystemInterface* system, const class UScene* uscene);

	~PhysicsScene();

	void deleteSystemObject(const class UObject* uobject) override;

	void tick(float deltaTime);

	const class UObject* raycast(
		const mat::vec3& origin,
		const mat::vec3& direction,
		mat::vec3& hit,
		const std::unordered_set<const class UObject*>& ignore) const;

private:

	std::list<std::unique_ptr<class PhysicsObject>> physicsObjects;

	SystemObjectInterface* addSystemObject(SystemObjectInterface* object) override;
};
