#pragma once

#include "GraphicsObject.h"
#include "../../Util/Matrix.h"
#include "../../Util/Observer.h"
#include <string>

class MeshGraphicsObject : public GraphicsObject, public ObserverInterface
{
public:

	MeshGraphicsObject(const class SystemSceneInterface* scene, const class UObject* uobject);

	~MeshGraphicsObject();

	void setMesh(std::string filepath);

	// Returns the world transform matrix of this component
	const mat::mat4& componentTransformMatrix() const;

	bool isSelected() const;

	bool bDirtyTransform;

	bool bDirtySelection;

private:

	// World space local location
	mat::vec3 position;

	// World space local rotation
	mat::vec3 rotation;

	// World space local scale
	mat::vec3 scale;

	// Global model matrix. This transform cumulates parent transforms
	mat::mat4 transformMatrix;

	// Shared mesh, storing rendering geometry
	class GMesh* mesh;

	// Object selection
	bool bSelected;
};
