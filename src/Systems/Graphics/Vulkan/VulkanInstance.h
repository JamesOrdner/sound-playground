#pragma once

#include <SDL_video.h>
#include <vulkan/vulkan.h>
#include <memory>
#include <vector>

class VulkanInstance
{
public:
	
	VulkanInstance(SDL_Window* window);
	
	~VulkanInstance();
	
private:
	
	VkInstance instance;
	VkSurfaceKHR surface;

	std::unique_ptr<class VulkanDevice> device;

	std::vector<const char*> requiredInstanceExtensions(SDL_Window* window);
};
