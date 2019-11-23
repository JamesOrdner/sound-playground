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

bool EWorld::raycast(const mat::vec3& origin, const mat::vec3& direction, mat::vec3& hitLoc, std::shared_ptr<EModel> hitObject)
{
	float shortest = -1;
	for (const auto& object : objects) {
		mat::vec3 hit;
		if (float l = object->raycast(origin, direction, hit) > 0) {
			if (shortest < 0 || l < shortest) {
				shortest = l;
				hitLoc = hit;
				hitObject = object;
			}
		}
	}
	return shortest > 0;
}
