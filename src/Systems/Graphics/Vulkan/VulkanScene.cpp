#include "VulkanScene.h"
#include "VulkanInstance.h"
#include "VulkanMaterial.h"
#include "VulkanMesh.h"
#include "VulkanFrame.h"
#include <algorithm>

VulkanScene::VulkanScene(VulkanInstance* instance) :
	vulkanInstance(instance)
{
	viewMatrix = mat::mat4::Identity();
	
	float aspectRatio = 720.f / 1280.f;
	projMatrix = mat::perspective(-0.05f, 0.05f, -0.05f * aspectRatio, 0.05f * aspectRatio, 0.1f, 15.f);
}

VulkanScene::~VulkanScene()
{
}

uint32_t modelID = 0; // TODO: TEMP
VulkanModel* VulkanScene::createModel()
{
	auto model = std::make_unique<VulkanModel>(this, modelID++);
	auto* ptr = model.get();
	models.push_back(std::move(model));
	sortModels();
	return ptr;
}

void VulkanScene::removeModel(VulkanModel* model)
{
	for (auto it = models.cbegin(); it != models.cend(); it++) {
		if (it->get() == model) {
			models.erase(it);
			break;
		}
	}
}

void VulkanScene::sortModels()
{
	std::sort(models.begin(), models.end(), [](auto& a, auto& b) {
		const auto* aMat = a->getMaterial();
		const auto* bMat = b->getMaterial();
		if (aMat && !bMat) return true;
		if ((!aMat && bMat) || (!aMat && !bMat)) return false;
		if (aMat->name != bMat->name) return aMat->name < bMat->name;
		
		const auto* aMesh = a->getMesh();
		const auto* bMesh = b->getMesh();
		if (aMesh && !bMesh) return true;
		if ((!aMesh && bMesh) || (!aMesh && !bMesh)) return false;
		return aMesh->filepath < bMesh->filepath;
	});
}

VulkanMesh* VulkanScene::modelMeshUpdated(const std::string& meshFilepath)
{
	auto* mesh = vulkanInstance->sharedMesh(meshFilepath);
	sortModels();
	return mesh;
}

VulkanMaterial* VulkanScene::modelMaterialUpdated(const std::string& materialName)
{
	auto* material = vulkanInstance->sharedMaterial(materialName);
	sortModels();
	return material;
}

void VulkanScene::setViewMatrix(const mat::mat4& matrix)
{
	viewMatrix = matrix;
}

void VulkanScene::setProjMatrix(const mat::mat4& matrix)
{
	projMatrix = matrix;
}

void VulkanScene::updateUniforms(const VulkanFrame& frame) const
{
	for (const auto& model : models) {
		frame.updateModelTransform(*model, viewMatrix);
	}
	
	frame.flushModelTransformUpdates();
	
	// TODO: Update only once
	frame.updateProjectionMatrix(projMatrix);
}

void VulkanScene::render(const VulkanFrame& frame) const
{
	VulkanMaterial* material = nullptr;
	VulkanMesh* mesh = nullptr;
	for (const auto& model : models) {
		if (material != model->getMaterial()) {
			material = model->getMaterial();
			if (material) frame.bindMaterial(*material);
		}
		
		if (mesh != model->getMesh()) {
			mesh = model->getMesh();
			if (mesh) frame.bindMesh(*mesh);
		}
		
		if (!material || !mesh) break;
		frame.draw(*model);
	}
}
