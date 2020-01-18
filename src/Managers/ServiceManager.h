#pragma once

#include "ServiceManagerInterface.h"

class ServiceManager : public ServiceManagerInterface
{
public:

	// PhysicsSystem services

	class PhysicsSystemInterface* physicsSystem;

	const class UObject* raycast(const class UScene* uscene, const mat::vec3& origin, const mat::vec3& direction, mat::vec3& hit) const override;

private:

	ServiceManager();

public:

	static ServiceManager& instance();

	// Deleted functions prevent singleton duplication
	ServiceManager(ServiceManager const&) = delete;
	void operator=(ServiceManager const&) = delete;
};
