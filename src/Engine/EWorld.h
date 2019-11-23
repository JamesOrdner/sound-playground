#pragma once

#include "../Graphics/Matrix.h"
#include <list>
#include <memory>

// Forward declarations
class EModel;

class EWorld
{
public:
	// Add an object to the world
	void addObject(const std::shared_ptr<EModel>& model);

	// Remove an object from the world
	void removeObject(const std::shared_ptr<EModel>& model);

	// Perform a raycast. Returns success.
	bool raycast(const mat::vec3& origin, const mat::vec3& direction, mat::vec3& hitLoc, std::shared_ptr<EModel> hitObject);

private:
	// All objects in this world
	std::list<std::shared_ptr<EModel>> objects;
};
