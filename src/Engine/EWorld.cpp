#include "EWorld.h"
#include "EObject.h"
#include "EModel.h"
#include "Engine.h"

void EWorld::addObject(const std::shared_ptr<EObject>& object)
{
	object->bExistsInWorld = true;
	objects.push_back(object);

	Engine& engine = Engine::instance();
	if (const auto& audioComponent = object->audioComponent()) {
		// Existing audio component will not be registered before adding to the world
		engine.audio().registerComponent(audioComponent);
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
	if (const auto& audioComponent = object->audioComponent()) {
		engine.audio().unregisterComponent(audioComponent);
	}

	if (const auto& model = std::dynamic_pointer_cast<EModel>(object)) {
		Engine::instance().unregisterModel(model);
	}
}

std::shared_ptr<EModel> EWorld::raycast(const mat::vec3& origin, const mat::vec3& direction, mat::vec3& hitLoc)
{
	float shortest = FLT_MAX;
	std::shared_ptr<EModel> hitObject;
	for (const auto& object : objects) {
		const auto& model = std::dynamic_pointer_cast<EModel>(object);
		if (!model) continue;
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
	return hitObject;
}
