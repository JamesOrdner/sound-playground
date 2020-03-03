#include "VulkanModel.h"
#include "VulkanScene.h"
#include "VulkanMesh.h"

VulkanModel::VulkanModel(VulkanScene* scene, uint32_t modelID) :
	modelID(modelID),
	transform(mat::mat4::Identity()),
	scene(scene),
	mesh(nullptr),
	material(nullptr)
{
}

void VulkanModel::setMesh(const std::string& filepath)
{
	mesh = scene->modelMeshUpdated(filepath);
}

void VulkanModel::setMaterial(const std::string& name)
{
	material = scene->modelMaterialUpdated(name);
}
