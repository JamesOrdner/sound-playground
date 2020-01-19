#include "MeshGraphicsObject.h"
#include "../../Engine/UObject.h"
#include "GMesh.h"

MeshGraphicsObject::MeshGraphicsObject(const SystemSceneInterface* scene, const UObject* uobject) :
	GraphicsObject(scene, uobject),
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
		[this](const EventData& data, bool bEventFromParent) {
			if (bEventFromParent) {
				mat::vec3 absPos = position + std::get<mat::vec3>(data);
				transformMatrix = mat::transform(absPos, rotation, scale);
				this->uobject->childEventImmediate(EventType::PositionUpdated, absPos);
			}
			else {
				position = std::get<mat::vec3>(data);
				transformMatrix = mat::transform(position, rotation, scale);
				this->uobject->childEventImmediate(EventType::PositionUpdated, position);
			}
			bDirtyTransform = true;
		}
	);

	registerCallback(
		uobject,
		EventType::RotationUpdated,
		[this](const EventData& data, bool bEventFromParent) {
			if (bEventFromParent) {
				mat::vec3 absRot = rotation + std::get<mat::vec3>(data);
				transformMatrix = mat::transform(position, absRot, scale);
				this->uobject->childEventImmediate(EventType::RotationUpdated, absRot);
			}
			else {
				rotation = std::get<mat::vec3>(data);
				transformMatrix = mat::transform(position, rotation, scale);
				this->uobject->childEventImmediate(EventType::RotationUpdated, rotation);
			}
			bDirtyTransform = true;
		}
	);

	registerCallback(
		uobject,
		EventType::ScaleUpdated,
		[this](const EventData& data, bool bEventFromParent) {
			if (bEventFromParent) {
				mat::vec3 absScale = scale * std::get<mat::vec3>(data);
				transformMatrix = mat::transform(position, rotation, absScale);
				this->uobject->childEventImmediate(EventType::ScaleUpdated, absScale);
			}
			else {
				scale = std::get<mat::vec3>(data);
				transformMatrix = mat::transform(position, rotation, scale);
				this->uobject->childEventImmediate(EventType::ScaleUpdated, scale);
			}
			bDirtyTransform = true;
		}
	);

	registerCallback(
		uobject,
		EventType::SelectionUpdated,
		[this](const EventData& data, bool bEventFromParent) {
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
