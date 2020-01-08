#include "EWorld.h"
#include "../Objects/EObject.h"
#include "../Objects/EModel.h"
#include "../Objects/ECamera.h"

EWorld::EWorld() :
	physicsUpdateInterval(0.05f),
	physicsDeltaTime(0.f)
{
	objects.emplace_front(std::make_unique<ECamera>());
	camera = static_cast<ECamera*>(objects.front().get());
}

EObject* EWorld::addObject(std::unique_ptr<EObject> object)
{
	EObject* objectPtr = object.get();
	objects.push_back(std::move(object));
	return objectPtr;
}

const std::list<std::unique_ptr<EObject>>& EWorld::allObjects() const
{
	return objects;
}

ECamera* EWorld::worldCamera() const
{
	return camera;
}

EModel* EWorld::raycast(
	const mat::vec3& origin,
	const mat::vec3& direction,
	mat::vec3& hitLoc,
	const std::unordered_set<EObject*>& ignore) const
{
	float shortest = FLT_MAX;
	EModel* hitObject = nullptr;
	for (const auto& object : objects) {
		if (ignore.find(object.get()) != ignore.end()) continue;
		if (EModel* model = dynamic_cast<EModel*>(object.get())) {
			mat::vec3 hit;
			float l = model->raycast(origin, direction, hit);
			if (l > 0) {
				if (!hitObject || l < shortest) {
					shortest = l;
					hitLoc = hit;
					hitObject = model;
				}
			}
		}
	}
	return hitObject;
}

void EWorld::tick(float deltaTime)
{
	physicsDeltaTime += deltaTime;
	while (physicsDeltaTime > physicsUpdateInterval) {
		for (const auto& object : objects) object->updatePhysics(physicsUpdateInterval);
		physicsDeltaTime -= physicsUpdateInterval;
	}

	for (const auto& object : objects) object->tick(deltaTime);
}
