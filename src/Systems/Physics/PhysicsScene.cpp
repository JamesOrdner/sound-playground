#include "PhysicsScene.h"
#include "PhysicsObject.h"

PhysicsScene::PhysicsScene(const SystemInterface* system, const UScene* uscene) :
	SystemSceneInterface(system, uscene)
{
}

PhysicsScene::~PhysicsScene()
{
	physicsObjects.clear();
}

void PhysicsScene::deleteSystemObject(const UObject* uobject)
{
	for (const auto& physicsObject : physicsObjects) {
		if (physicsObject->uobject == uobject) {
			physicsObjects.remove(physicsObject);
			break;
		}
	}
}

void PhysicsScene::tick(float deltaTime)
{
	for (const auto& physicsObject : physicsObjects) {
		physicsObject->updateVelocity(deltaTime);
	}
}

const UObject* PhysicsScene::raycast(
	const mat::vec3& origin,
	const mat::vec3& direction,
	mat::vec3& hit,
	const std::unordered_set<const UObject*>& ignore) const
{
	float shortest = FLT_MAX;
	const UObject* hitObject = nullptr;
	for (const auto& obj : physicsObjects) {
		if (ignore.count(obj->uobject) > 0) continue;
		mat::vec3 objHit;
		float l = obj->raycast(origin, direction, objHit);
		if (l > 0) {
			if (!hitObject || l < shortest) {
				shortest = l;
				hit = objHit;
				hitObject = obj->uobject;
			}
		}
	}
	return hitObject;
}

SystemObjectInterface* PhysicsScene::addSystemObject(SystemObjectInterface* object)
{
	physicsObjects.emplace_back(static_cast<PhysicsObject*>(object));
	return object;
}
