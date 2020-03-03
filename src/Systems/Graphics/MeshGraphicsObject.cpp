#include "MeshGraphicsObject.h"
#include "../../Engine/UObject.h"
#include "Vulkan/VulkanModel.h"

MeshGraphicsObject::MeshGraphicsObject(const SystemSceneInterface* scene, const UObject* uobject) :
	GraphicsObject(scene, uobject),
	bDirtyTransform(true),
	bDirtySelection(true),
	model(nullptr),
	scale(1),
	parentScale(1),
	bSelected(false)
{
	registerCallback(
		uobject,
		EventType::PositionUpdated,
		[this](const EventData& data, bool bEventFromParent) {
			(bEventFromParent ? parentPosition : position) = std::get<mat::vec3>(data);
			this->uobject->childEventImmediate(EventType::PositionUpdated, position + parentPosition);
			bDirtyTransform = true;
			// TODO: TEMP
			model->transform = mat::t(transformMatrix());
		}
	);

	registerCallback(
		uobject,
		EventType::RotationUpdated,
		[this](const EventData& data, bool bEventFromParent) {
			(bEventFromParent ? parentRotation : rotation) = std::get<mat::vec3>(data);
			this->uobject->childEventImmediate(EventType::RotationUpdated, rotation + parentRotation);
			bDirtyTransform = true;
		}
	);

	registerCallback(
		uobject,
		EventType::ScaleUpdated,
		[this](const EventData& data, bool bEventFromParent) {
			(bEventFromParent ? parentScale : scale) = std::get<mat::vec3>(data);
			this->uobject->childEventImmediate(EventType::ScaleUpdated, scale * parentScale);
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
}

void MeshGraphicsObject::setMesh(std::string filepath)
{
	model->setMesh(filepath);
	model->setMaterial("main"); // TEMP
}

const mat::mat4& MeshGraphicsObject::transformMatrix()
{
	if (bDirtyTransform) {
		transform = mat::transform(position + parentPosition, rotation + parentRotation, scale * parentScale);
		bDirtyTransform = false;
	}
	return transform;
}

bool MeshGraphicsObject::isSelected() const
{
	return bSelected;
}
