#pragma once

#include "../../../Util/Matrix.h"
#include <string>

class VulkanModel
{
public:
	
	VulkanModel(class VulkanScene* scene);
	
	inline class VulkanMesh* getMesh() const { return mesh; }
	inline class VulkanMaterial* getMaterial() const { return material; }
	
	void setMesh(const std::string& filepath);
	
	void setMaterial(const std::string& name);
	
private:
	
	class VulkanScene* const scene;
	
	class VulkanMesh* mesh;
	
	class VulkanMaterial* material;
	
	mat::mat4 modelTransform;
};
