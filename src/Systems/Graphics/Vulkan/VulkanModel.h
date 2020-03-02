#pragma once

#include "../../../Util/Matrix.h"

class VulkanModel
{
public:
	
	VulkanModel();
	
	inline class VulkanMesh* getMesh() const { return mesh; }
	inline class VulkanMaterial* getMaterial() const { return material; }
	
private:
	
	class VulkanMesh* mesh;
	
	class VulkanMaterial* material;
	
	mat::mat4 modelTransform;
};
