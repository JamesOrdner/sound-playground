#include "MeshGraphicsObject.h"
#include "../Managers/StateManager.h"
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
	auto& stateManager = StateManager::instance();

	observerIDs.push_back(
		stateManager.registerObserver(
			uobject,
			StateManager::EventType::PositionUpdated,
			[this](const StateManager::EventData& data) {
				position = std::get<mat::vec3>(data);
				transformMatrix = mat::transform(position, rotation, scale);
				bDirtyTransform = true;
			}
		)
	);

	observerIDs.push_back(
		stateManager.registerObserver(
			uobject,
			StateManager::EventType::RotationUpdated,
			[this](const StateManager::EventData& data) {
				rotation = std::get<mat::vec3>(data);
				transformMatrix = mat::transform(position, rotation, scale);
				bDirtyTransform = true;
			}
		)
	);

	observerIDs.push_back(
		stateManager.registerObserver(
			uobject,
			StateManager::EventType::ScaleUpdated,
			[this](const StateManager::EventData& data) {
				scale = std::get<mat::vec3>(data);
				transformMatrix = mat::transform(position, rotation, scale);
				bDirtyTransform = true;
			}
		)
	);

	observerIDs.push_back(
		stateManager.registerObserver(
			uobject,
			StateManager::EventType::SelectionUpdated,
			[this](const StateManager::EventData& data) {
				bSelected = std::get<bool>(data);
				bDirtySelection = true;
			}
		)
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
