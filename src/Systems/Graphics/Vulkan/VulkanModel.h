#pragma once

#include "../../../Util/Matrix.h"

class VulkanModel
{
public:
	
	VulkanModel();
	
private:
	
	class VulkanMesh* mesh;
	
	mat::mat4 modelTransform;
};
