#include "ServiceManager.h"
#include "../Systems/Graphics/GraphicsSystemInterface.h"
#include "../Systems/Physics/PhysicsSystemInterface.h"

ServiceManager& ServiceManager::instance()
{
	static ServiceManager instance;
	return instance;
}

ServiceManager::ServiceManager() :
	graphicsSystem(nullptr),
	physicsSystem(nullptr)
{
}

void ServiceManager::screenDimensions(int& x, int& y) const
{
	graphicsSystem->screenDimensions(x, y);
}

const mat::mat4& ServiceManager::screenToWorldTransform(const UScene* uscene) const
{
	return graphicsSystem->screenToWorldTransform(uscene);
}

const UObject* ServiceManager::raycast(
	const UScene* uscene,
	const mat::vec3& origin,
	const mat::vec3& direction,
	const std::unordered_set<const UObject*>& ignore) const
{
	mat::vec3 hit;
	return raycast(uscene, origin, direction, hit, ignore);
}

const UObject* ServiceManager::raycast(
	const UScene* uscene,
	const mat::vec3& origin,
	const mat::vec3& direction,
	mat::vec3& hit,
	const std::unordered_set<const UObject*>& ignore) const
{
	return physicsSystem->raycast(uscene, origin, direction, hit, ignore);
}

const UObject* ServiceManager::raycastScreen(
	const UScene* uscene,
	int x,
	int y,
	const std::unordered_set<const UObject*>& ignore) const
{
	mat::vec3 hit;
	return raycastScreen(uscene, x, y, hit, ignore);
}

const UObject* ServiceManager::raycastScreen(
	const UScene* uscene,
	int x,
	int y,
	mat::vec3& hit,
	const std::unordered_set<const UObject*>& ignore) const
{
	int width, height;
	screenDimensions(width, height);

	mat::vec4 rayStartScreen{
		static_cast<float>(x - width / 2) / (width / 2),
		static_cast<float>(height / 2 - y) / (height / 2),
		-1.f,
		1.f };
	mat::vec4 rayStartWorld = screenToWorldTransform(uscene) * rayStartScreen;
	mat::vec3 origin = mat::vec3(rayStartWorld) / rayStartWorld.w;

	mat::vec4 rayEndScreen{
		static_cast<float>(x - width / 2) / (width / 2),
		static_cast<float>(y - height / 2) / (height / 2),
		1.f,
		1.f };
	mat::vec4 rayEndWorld = screenToWorldTransform(uscene) * rayEndScreen;
	mat::vec3 direction = mat::vec3(rayEndWorld) / rayEndWorld.w - origin;

	return raycast(uscene, origin, direction, hit, ignore);
}
