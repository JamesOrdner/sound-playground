#pragma once

#include "GraphicsObject.h"
#include "../Util/Matrix.h"
#include <string>

class MeshGraphicsObject : public GraphicsObject
{
public:

	MeshGraphicsObject(const class UObject* uobject);

	~MeshGraphicsObject();

	void setMesh(std::string filepath);

	// Returns the world transform matrix of this component
	const mat::mat4& componentTransformMatrix() const;

	bool isSelected() const;

	bool bDirtyTransform;

	bool bDirtySelection;

private:

	// World space location
	mat::vec3 position;

	// World space Euler rotation
	mat::vec3 rotation;

	// World space scale
	mat::vec3 scale;

	// Model matrix
	mat::mat4 transformMatrix;

	// Shared mesh, storing rendering geometry
	class GMesh* mesh;

	// Object selection
	bool bSelected;
};
