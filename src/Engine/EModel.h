#pragma once

#include "EObject.h"
#include <string>

// Forward declarations
class GMesh;

class EModel : public EObject
{
public:

	EModel();

	virtual ~EModel();

	// Is this object being actively placed? Object tracks mouse cursor when true.
	bool bActivePlacement;

	// Set the visual representation of the model via a mesh filepath
	void setMesh(std::string filepath);

	// Perform a worldspace raycast against this model.
	// Returns the length of the hit ray (negative if no hit).
	float raycast(const mat::vec3& origin, const mat::vec3& direction, mat::vec3& hitLoc);

	// Returns the world transform matrix for this model
	mat::mat4 transformMatrix() const;

	// Returns true if transform has been modified and needs render update
	bool needsTransformUpdate();

	// Called once transform updates have been copied to rendering device
	void transformUpdated();

	// Returns true if model selection has been modified and needs GPU update
	bool needsSelectionUpdate();

	// Called once selection updates have been copied to GPU
	void selectionUpdated();

	// EObject interface
	void setSelected(bool selected) override;
	void setPosition(const mat::vec3& location) override;
	void setRotation(const mat::vec3& rotation) override;
	void setScale(float scale) override;
	void setScale(const mat::vec3& scale) override;
	void tick(float deltaTime) override;
	void updatePhysics(float deltaTime) override;

private:

	// Cursor coordinates, updated when bActivePlacement == true
	int mouseX, mouseY;

	// Shared mesh, storing geometry and other rendering data
	GMesh* mesh;

	// Set to true when location, rotation, or scale is modified
	bool bDirtyTransform;

	// Set to true when model selection is modified
	bool bDirtySelection;

	/** Physics */

	// Position of the model after most recent physics update
	mat::vec3 physicsPosition;
};
