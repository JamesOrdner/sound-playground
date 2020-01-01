#include "EWorld.h"
#include "EObject.h"
#include "EModel.h"
#include "Engine.h"
#include "../Audio/AudioEngine.h"

EWorld::EWorld() :
	physicsUpdateMinInterval(0.05f),
	physicsDeltaTime(0.f)
{
}

void EWorld::addObject(const std::shared_ptr<EObject>& object)
{
	object->bExistsInWorld = true;
	objects.push_back(object);

	Engine& engine = Engine::instance();
	if (auto* audioComponent = object->audioComponent()) {
		// Existing audio component will not be registered before adding to the world
		engine.audio().registerComponent(audioComponent, object.get());
	}

	if (const auto& model = std::dynamic_pointer_cast<EModel>(object)) {
		// Models require registration with the engine
		engine.registerModel(model);
	}
}

void EWorld::removeObject(const std::shared_ptr<EObject>& object)
{
	object->bExistsInWorld = false;
	objects.remove(object);

	Engine& engine = Engine::instance();
	if (auto* audioComponent = object->audioComponent()) {
		engine.audio().unregisterComponent(audioComponent);
	}

	if (const auto& model = std::dynamic_pointer_cast<EModel>(object)) {
		Engine::instance().unregisterModel(model);
	}
}

const std::list<std::shared_ptr<EObject>>& EWorld::allObjects() const
{
	return objects;
}

EModel* EWorld::raycast(const mat::vec3& origin, const mat::vec3& direction, mat::vec3& hitLoc)
{
	float shortest = FLT_MAX;
	EModel* hitObject = nullptr;
	for (const auto& object : objects) {
		const auto& model = std::dynamic_pointer_cast<EModel>(object);
		if (!model) continue;
		mat::vec3 hit;
		float l = model->raycast(origin, direction, hit);
		if (l > 0) {
			if (!hitObject || l < shortest) {
				shortest = l;
				hitLoc = hit;
				hitObject = model.get();
			}
		}
	}
	return hitObject;
}

void EWorld::tick(float deltaTime)
{
	physicsDeltaTime += deltaTime;
	if (physicsDeltaTime > physicsUpdateMinInterval) {
		for (const auto& object : objects) object->updatePhysics(physicsDeltaTime);
		physicsDeltaTime = 0.f;
	}
}
