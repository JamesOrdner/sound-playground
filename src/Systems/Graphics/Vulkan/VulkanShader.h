#pragma once

#include <vulkan/vulkan.h>
#include <string>
#include <vector>

/// VulkanShader creates a vertex VkShaderModule and a fragment VkShaderModule.
/// These will be destroyed automatically when the VulkanShader object is detroyed.
class VulkanShader
{
public:

	/// Pass the name of the vert/frag shader file pair, without file extension.
	/// Shader files must reside in the res/shaders/ directory have the extensions .vert.spv and .frag.spv
	VulkanShader(VkDevice device, const std::string& filename);

	~VulkanShader();

	const std::string filename;

	VkShaderModule vertShaderModule;
	VkShaderModule fragShaderModule;

	std::vector<VkPipelineShaderStageCreateInfo> stages;

private:

	const VkDevice device;

	std::vector<char> readShaderFile(const std::string& filepath);
	VkShaderModule createShaderModule(const std::vector<char>& code);
};
