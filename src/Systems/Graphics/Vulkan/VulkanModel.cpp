#include "VulkanModel.h"
#include "VulkanMesh.h"

VulkanModel::VulkanModel() :
	modelTransform(mat::mat4::Identity()),
	mesh(nullptr),
	material(nullptr)
{
}
