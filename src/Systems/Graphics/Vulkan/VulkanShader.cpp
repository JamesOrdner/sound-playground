#include "VulkanShader.h"
#include <stdexcept>
#include <fstream>
#include <filesystem>

VulkanShader::VulkanShader(VkDevice device, const std::string& filename) :
	filename(filename),
	vertShaderModule(VK_NULL_HANDLE),
	fragShaderModule(VK_NULL_HANDLE),
    device(device)
{
	std::string vertShaderPath = "res/shaders/" + filename + ".vert.spv";
	if (std::filesystem::exists(vertShaderPath)) {
		auto shaderCode = readShaderFile(vertShaderPath);
		vertShaderModule = createShaderModule(shaderCode);
		stages.push_back({
			.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
			.stage = VK_SHADER_STAGE_VERTEX_BIT,
			.module = vertShaderModule,
			.pName = "main"
		});
	}
	
	std::string fragShaderPath = "res/shaders/" + filename + ".frag.spv";
	if (std::filesystem::exists(fragShaderPath)) {
		auto shaderCode = readShaderFile(fragShaderPath);
		fragShaderModule = createShaderModule(shaderCode);
		stages.push_back({
			.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
			.stage = VK_SHADER_STAGE_FRAGMENT_BIT,
			.module = fragShaderModule,
			.pName = "main"
		});
	}
}

VulkanShader::~VulkanShader()
{
	if (vertShaderModule) vkDestroyShaderModule(device, vertShaderModule, nullptr);
	if (fragShaderModule) vkDestroyShaderModule(device, fragShaderModule, nullptr);
}

std::vector<char> VulkanShader::readShaderFile(const std::string& filepath)
{
	std::ifstream file(filepath, std::ios::ate | std::ios::binary);
	if (!file.is_open()) {
		throw std::runtime_error("Failed to open Vulkan shader file!");
	}
	
	size_t fileSize = (size_t)file.tellg();
	std::vector<char> buffer(fileSize);
	file.seekg(0);
	file.read(buffer.data(), fileSize);
	file.close();
	return buffer;
}

VkShaderModule VulkanShader::createShaderModule(const std::vector<char>& code)
{
	VkShaderModule shaderModule;
	
	VkShaderModuleCreateInfo shaderModuleInfo{
		.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
		.codeSize = code.size(),
		.pCode = reinterpret_cast<const uint32_t*>(code.data())
	};
	
	if (vkCreateShaderModule(device, &shaderModuleInfo, nullptr, &shaderModule) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create Vulkan shader module!");
	}
	
	return shaderModule;
}
