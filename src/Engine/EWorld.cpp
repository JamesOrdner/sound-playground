#include "EWorld.h"
#include "EModel.h"
#include "Engine.h"

void EWorld::addObject(const std::shared_ptr<EModel>& model)
{
	objects.push_back(model);
	Engine::instance().registerModel(model);
}

void EWorld::removeObject(const std::shared_ptr<EModel>& model)
{
	objects.remove(model);
	Engine::instance().unregisterModel(model);
}

std::shared_ptr<EModel> EWorld::raycast(const mat::vec3& origin, const mat::vec3& direction, mat::vec3& hitLoc)
{
	float shortest = FLT_MAX;
	std::shared_ptr<EModel> hitObject;
	for (const auto& object : objects) {
		mat::vec3 hit;
		float l = object->raycast(origin, direction, hit);
		if (l > 0) {
			if (!hitObject || l < shortest) {
				shortest = l;
				hitLoc = hit;
				hitObject = object;
			}
		}
	}
	return hitObject;
}
