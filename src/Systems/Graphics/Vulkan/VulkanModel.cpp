#include "VulkanModel.h"
#include "VulkanScene.h"
#include "VulkanMesh.h"

VulkanModel::VulkanModel(VulkanScene* scene, uint32_t modelID) :
	modelID(modelID),
	scene(scene),
	mesh(nullptr),
	material(nullptr),
	modelTransform(mat::mat4::Identity())
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
