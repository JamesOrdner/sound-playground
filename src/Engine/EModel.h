#pragma once

#include "EObject.h"

#include <memory>
#include <string>
#include <vector>

// Forward declarations
class GMesh;

class EModel : public EObject
{
public:
	EModel(const std::string& filepath);

	// Returns the path to the model file
	std::string getFilepath();

	// Called from the engine upon registration
	void registerWithMesh(std::shared_ptr<GMesh> mesh);

	// Called from engine upon removal of registration
	void unregister();

	// Returns the mesh associated with this model
	std::shared_ptr<GMesh> getMesh();

	// Perform a raycast against this model. Returns the length of the hit ray (negative if no hit).
	float raycast(const mat::vec3& origin, const mat::vec3& direction, mat::vec3& hitLoc);

	// Returns the world transform matrix for this model
	mat::mat4 transformMatrix();

	// Returns true if transform has been modified and needs render update
	bool needsTransformUpdate();

	// Called once transform updates have been copied to rendering device
	void transformUpdated();

	/** EObject overrides */

	// Sets the world space position of the model
	void setPosition(const mat::vec3& location) override;

	// Sets the uniform scale of the model
	void setScale(float scale) override;

	// Sets the scale of the model
	void setScale(const mat::vec3& scale) override;

private:

	// Filepath of the model's mesh
	std::string filepath;

	// Mesh storing geometry and other rendering data
	std::shared_ptr<GMesh> mesh;

	// Set to true when location, rotation, or scale is modified
	bool bDirtyTransform;
};
