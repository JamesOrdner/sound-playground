#include "VulkanScene.h"
#include "VulkanMaterial.h"
#include "VulkanModel.h"
#include "VulkanMesh.h"
#include "VulkanFrame.h"
#include <vector>

struct VulkanSceneData
{
	/// Models are sorted by material and mesh for efficient rendering iteration
	std::vector<VulkanModel> models;
	std::vector<VulkanMesh> meshes;
	std::vector<VulkanMaterial> materials;
	
	void clear() {
		models.clear();
		meshes.clear();
		materials.clear();
	}
};

VulkanScene::VulkanScene() :
	data(std::make_unique<VulkanSceneData>())
{
}

VulkanScene::~VulkanScene()
{
}

void VulkanScene::updateUniforms(const VulkanFrame& frame) const
{
	for (const auto& model : data->models) {
		// frame.updateTransformUniform(model.uniformAddress, model.transform);
	}
}

void VulkanScene::render(const VulkanFrame& frame) const
{
	VulkanMaterial* material = nullptr;
	VulkanMesh* mesh = nullptr;
	for (const auto& model : data->models) {
		if (material != model.getMaterial()) {
			material = model.getMaterial();
			frame.bindMaterial(*material);
		}
		
		if (mesh != model.getMesh()) {
			mesh = model.getMesh();
			frame.bindMesh(*mesh);
		}
		
		frame.draw(model);
	}
}
