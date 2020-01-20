#pragma once

#include "../SystemInterface.h"
#include "PhysicsSystemInterface.h"
#include <list>
#include <memory>

class PhysicsSystem : public SystemInterface, public PhysicsSystemInterface
{
public:

	PhysicsSystem();

	~PhysicsSystem();

	// SystemInterface
	bool init() override;
	void deinit() override;
	void execute(float deltaTime) override;
	SystemSceneInterface* createSystemScene(const class UScene* uscene) override;
	SystemSceneInterface* findSystemScene(const class UScene* uscene) override;

	// PhysicsSystemInterface
	const class UObject* raycast(
		const class UScene* uscene,
		const mat::vec3& origin,
		const mat::vec3& direction,
		mat::vec3& hit,
		const std::unordered_set<const class UObject*>& ignore) const override;

private:

	std::list<std::unique_ptr<class PhysicsScene>> physicsScenes;
};
