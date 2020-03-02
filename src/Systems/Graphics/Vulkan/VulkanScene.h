#pragma once

#include <vector>
#include <memory>

class VulkanScene
{
public:
	
	VulkanScene();
	
	~VulkanScene();
	
	/// Update uniform buffers of all models in the scene.
	/// Called prior to beginning the render pass.
	void updateUniforms(const class VulkanFrame& frame) const;
	
	/// Draw all models in the scene. Called after beginning the render pass.
	void render(const class VulkanFrame& frame) const;
	
private:
	
	/// Models are sorted first by material, and then by mesh, for fast render iteration.
	/// Models without a registered mesh or material are stored at the end of the vector.
	std::vector<std::unique_ptr<class VulkanModel>> models;
};
