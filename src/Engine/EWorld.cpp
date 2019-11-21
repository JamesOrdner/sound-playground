#include "EWorld.h"
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
