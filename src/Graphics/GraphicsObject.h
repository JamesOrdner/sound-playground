#pragma once

#include "../Engine/SystemObjectInterface.h"
#include "Matrix.h"
#include <string>

class GraphicsObject : public SystemObjectInterface
{
public:

	GraphicsObject(const class UObject* uobject);

	~GraphicsObject();

	void setMesh(std::string filepath);

	// Returns the world transform matrix of this component
	const mat::mat4& componentTransformMatrix() const;

	bool isSelected() const;

	bool bDirtyTransform;

	bool bDirtySelection;

private:

	// Shared mesh, storing rendering geometry
	class GMesh* mesh;

	// World space location
	mat::vec3 position;

	// World space Euler rotation
	mat::vec3 rotation;

	// World space scale
	mat::vec3 scale;

	// Model matrix
	mat::mat4 transformMatrix;

	// Object selection
	bool bSelected;
};
