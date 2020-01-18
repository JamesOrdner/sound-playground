#pragma once

#include "ServiceManagerInterface.h"

class ServiceManager : public ServiceManagerInterface
{
public:

	// GraphicsSystem services

	class GraphicsSystemInterface* graphicsSystem;

	void screenDimensions(int& x, int& y) const override;
	const mat::mat4& screenToWorldTransform(const class UScene* uscene) const;

	// PhysicsSystem services

	class PhysicsSystemInterface* physicsSystem;

	const class UObject* raycast(const class UScene* uscene, const mat::vec3& origin, const mat::vec3& direction) const override;
	const class UObject* raycast(const class UScene* uscene, const mat::vec3& origin, const mat::vec3& direction, mat::vec3& hit) const override;
	const class UObject* raycastScreen(const class UScene* uscene, int x, int y) const override;
	const class UObject* raycastScreen(const class UScene* uscene, int x, int y, mat::vec3& hit) const override;

private:

	ServiceManager();

public:

	static ServiceManager& instance();

	// Deleted functions prevent singleton duplication
	ServiceManager(ServiceManager const&) = delete;
	void operator=(ServiceManager const&) = delete;
};
