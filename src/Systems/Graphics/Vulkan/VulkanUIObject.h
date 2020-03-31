#pragma once

#include "../../../Util/Matrix.h"
#include <string>

class VulkanUIObject
{
public:
	
	VulkanUIObject(class VulkanInstance* instance);
	
	void setTexture(const std::string& filepath);
	
	inline class VulkanTexture* getTexture() const { return texture; }
	
	// all in NDC
	mat::vec2 position;
	mat::vec2 bounds;
	mat::vec2 uv_position;
	mat::vec2 uv_bounds;
	
	// draws from low to high
	uint32_t drawOrder;
	
private:
	
	class VulkanInstance* const instance;
	
	class VulkanTexture* texture;
};
