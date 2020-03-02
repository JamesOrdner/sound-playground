#pragma once

#include <vector>
#include <memory>
#include <string>

class VulkanScene
{
public:
	
	VulkanScene();
	
	~VulkanScene();
	
	/// Add a new model to the scene
	class VulkanModel* createModel();
	
	/// Remove a model from the scene
	void removeModel(class VulkanModel* model);
	
	/// Update uniform buffers of all models in the scene.
	/// Called prior to beginning the render pass.
	void updateUniforms(const class VulkanFrame& frame) const;
	
	/// Draw all models in the scene. Called after beginning the render pass.
	void render(const class VulkanFrame& frame) const;
	
private:
	
	/// Models are sorted first by material, and then by mesh, for fast render iteration.
	/// Models without a registered mesh or material are stored at the end of the vector.
	std::vector<std::unique_ptr<class VulkanModel>> models;
	
	void sortModels();
	
	friend class VulkanModel;
	
	/// Returns a shared pointer to the specified mesh and re-sorts the models array
	class VulkanMesh* modelMeshUpdated(const std::string& meshFilepath);
	
	/// Returns a shared pointer to the specified material and re-sorts the models array
	class VulkanMaterial* modelMaterialUpdated(const std::string& materialName);
};
