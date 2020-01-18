#include "ServiceManager.h"
#include "../Systems/Physics/PhysicsSystemInterface.h"

ServiceManager& ServiceManager::instance()
{
	static ServiceManager instance;
	return instance;
}


ServiceManager::ServiceManager() :
	physicsSystem(nullptr)
{
}

const UObject* ServiceManager::raycast(const UScene* uscene, const mat::vec3& origin, const mat::vec3& direction, mat::vec3& hit) const
{
	return physicsSystem->raycast(uscene, origin, direction, hit);
}
