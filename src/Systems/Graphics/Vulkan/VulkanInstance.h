#pragma once

#include <vulkan/vulkan.h>
#include <vector>

class VulkanInstance
{
public:
	
	VulkanInstance(struct SDL_Window* window);
	
	~VulkanInstance();
	
private:
	
	VkInstance instance;
	VkSurfaceKHR surface;
	
	void initInstance();
};
