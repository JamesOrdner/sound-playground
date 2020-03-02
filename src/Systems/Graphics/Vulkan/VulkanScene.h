#pragma once

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
	
	std::unique_ptr<struct VulkanSceneData> data;
};
