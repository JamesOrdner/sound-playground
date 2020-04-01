#pragma once

#include "../../../Util/Matrix.h"
#include <string>

class VulkanModel
{
public:
	
	VulkanModel(class VulkanScene* scene, uint32_t modelID);
	
	inline class VulkanMesh* getMesh() const { return mesh; }
	inline class VulkanMaterial* getMaterial() const { return material; }
	
	void setMesh(const std::string& filepath);
	void setMaterial(const std::string& name);
	
	/// A unique int ID used for indexing into the uniform buffer
	const uint32_t modelID;
	
	mat::mat4 transform;
	
	bool bSelected;
	
private:
	
	class VulkanScene* const scene;
	
	class VulkanMesh* mesh;
	
	class VulkanMaterial* material;
};
