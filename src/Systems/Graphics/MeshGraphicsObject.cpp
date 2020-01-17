#include "MeshGraphicsObject.h"
#include "../../Engine/UObject.h"
#include "GMesh.h"

MeshGraphicsObject::MeshGraphicsObject(const UObject* uobject) :
	GraphicsObject(uobject),
	bDirtyTransform(true),
	bDirtySelection(true),
	mesh(nullptr),
	scale(1),
	transformMatrix(mat::mat4::Identity()),
	bSelected(false)
{
	registerCallback(
		uobject,
		EventType::PositionUpdated,
		[this](const EventData& data) {
			position = std::get<mat::vec3>(data);
			transformMatrix = mat::transform(position, rotation, scale);
			bDirtyTransform = true;
		}
	);

	registerCallback(
		uobject,
		EventType::RotationUpdated,
		[this](const EventData& data) {
			rotation = std::get<mat::vec3>(data);
			transformMatrix = mat::transform(position, rotation, scale);
			bDirtyTransform = true;
		}
	);

	registerCallback(
		uobject,
		EventType::ScaleUpdated,
		[this](const EventData& data) {
			scale = std::get<mat::vec3>(data);
			transformMatrix = mat::transform(position, rotation, scale);
			bDirtyTransform = true;
		}
	);

	registerCallback(
		uobject,
		EventType::SelectionUpdated,
		[this](const EventData& data) {
			bSelected = std::get<bool>(data);
			bDirtySelection = true;
		}
	);
}

MeshGraphicsObject::~MeshGraphicsObject()
{
	if (mesh) mesh->unregisterWithComponent(this);
}

void MeshGraphicsObject::setMesh(std::string filepath)
{
	if (mesh) mesh->unregisterWithComponent(this);
	mesh = GMesh::getSharedMesh(filepath);
	mesh->registerWithComponent(this);
}

const mat::mat4& MeshGraphicsObject::componentTransformMatrix() const
{
	return transformMatrix;
}

bool MeshGraphicsObject::isSelected() const
{
	return bSelected;
}
