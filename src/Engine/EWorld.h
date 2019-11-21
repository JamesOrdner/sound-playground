#pragma once

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

private:
	// All objects in this world
	std::list<std::shared_ptr<EModel>> objects;
};
