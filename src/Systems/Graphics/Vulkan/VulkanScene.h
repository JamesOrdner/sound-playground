#pragma once

#include "VulkanModel.h"
#include <vector>
#include <memory>
#include <string>

class VulkanScene
{
public:
	
	VulkanScene(class VulkanInstance* instance);
	
	~VulkanScene();
	
	/// Add a new model to the scene
	class VulkanModel* createModel();
	
	/// Remove a model from the scene
	void removeModel(class VulkanModel* model);
	
	void setViewMatrix(const mat::mat4& matrix);
	void setProjMatrix(const mat::mat4& matrix);
	
	/// Update uniform buffers of all models in the scene.
	/// Called prior to beginning the render pass.
	void updateUniforms(const class VulkanFrame& frame) const;
	
	/// Draw all models in the scene. Called after beginning the render pass.
	void render(const class VulkanFrame& frame) const;
	
private:
	
	class VulkanInstance* const vulkanInstance;
	
	mat::mat4 viewMatrix, projMatrix;
	
	/// Models are sorted first by material, and then by mesh, for fast render iteration.
	/// Models without a registered mesh or material are stored at the end of the vector.
	std::vector<std::unique_ptr<class VulkanModel>> models;
	
	void sortModels();
	
	/// Returns a shared pointer to the specified mesh and re-sorts the models array
	class VulkanMesh* modelMeshUpdated(const std::string& meshFilepath);
	friend void VulkanModel::setMesh(const std::string& filepath);
	
	/// Returns a shared pointer to the specified material and re-sorts the models array
	class VulkanMaterial* modelMaterialUpdated(const std::string& materialName);
	friend void VulkanModel::setMaterial(const std::string& name);
};
