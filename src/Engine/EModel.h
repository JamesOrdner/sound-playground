#pragma once

#include "../Graphics/Matrix.h"
#include "../Graphics/GMesh.h"
#include <memory>
#include <string>

class EModel
{
public:
	EModel(const std::string& filepath);

	// Name used to identify this mesh
	std::string name;

	// Returns the path of the mesh associated with this model
	std::string meshFilepath();

	/** Location setters and getters */

	void setLocation(const mat::vec3& location);
	const mat::vec3& getLocation();

	/** Scale setters and getters */

	void setScale(float scale);
	void setScale(const mat::vec3& scale);
	const mat::vec3& getScale();

	/** Rendering */

	void draw(unsigned int modelMatrixID);

private:
	std::shared_ptr<GMesh> mesh;

	// World space location
	mat::vec3 location;

	// World space scale
	mat::vec3 scale;

	// modelMatrix updates only if location, rotation, or scale are modified
	mat::mat4 modelMatrix;
};

