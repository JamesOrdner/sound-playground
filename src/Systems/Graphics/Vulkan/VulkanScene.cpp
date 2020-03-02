#include "VulkanScene.h"
#include "VulkanMaterial.h"
#include "VulkanModel.h"
#include "VulkanMesh.h"
#include "VulkanFrame.h"

VulkanScene::VulkanScene()
{
}

VulkanScene::~VulkanScene()
{
}

void VulkanScene::updateUniforms(const VulkanFrame& frame) const
{
	for (const auto& model : models) {
		// frame.updateTransformUniform(model.uniformAddress, model.transform);
	}
}

void VulkanScene::render(const VulkanFrame& frame) const
{
	VulkanMaterial* material = nullptr;
	VulkanMesh* mesh = nullptr;
	for (const auto& model : models) {
		if (material != model->getMaterial()) {
			material = model->getMaterial();
			if (!material) break;
			frame.bindMaterial(*material);
		}
		
		if (mesh != model->getMesh()) {
			mesh = model->getMesh();
			if (!mesh) break;
			frame.bindMesh(*mesh);
		}
		
		frame.draw(*model);
	}
}
