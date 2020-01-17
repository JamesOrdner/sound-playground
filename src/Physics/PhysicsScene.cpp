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

UObject* PhysicsScene::raycast(const mat::vec3& origin, const mat::vec3& direction, mat::vec3& hit)
{
	return nullptr;
}

SystemObjectInterface* PhysicsScene::addSystemObject(SystemObjectInterface* object)
{
	physicsObjects.emplace_back(static_cast<PhysicsObject*>(object));
	return object;
}
