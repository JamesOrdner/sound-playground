#pragma once

#include "../Graphics/Matrix.h"

#include <memory>
#include <string>
#include <vector>

// Forward declarations
class GMesh;

class EModel
{
public:
	EModel(const std::string& filepath);

	// Name used to identify this mesh
	std::string name;

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

	/** Location setters and getters */

	void setPosition(const mat::vec3& location);
	const mat::vec3& getPosition();

	/** Scale setters and getters */

	void setScale(float scale);
	void setScale(const mat::vec3& scale);
	const mat::vec3& getScale();

	// Returns the world transform matrix for this model
	mat::mat4 transformMatrix();

	// Returns true if transform has been modified and needs render update
	bool needsTransformUpdate();

	// Called once transform updates have been copied to rendering device
	void transformUpdated();

private:

	// Filepath of the model's mesh
	std::string filepath;

	// Mesh storing geometry and other rendering data
	std::shared_ptr<GMesh> mesh;

	// World space location
	mat::vec3 position;

	// World space scale
	mat::vec3 scale;

	// Set to true when location, rotation, or scale is modified
	bool bDirtyTransform;
};
