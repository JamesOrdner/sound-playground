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
	
	inline class VulkanDevice* getDevice() const { return device.get(); }
	
private:
	
	VkInstance instance;
	VkSurfaceKHR surface;

	std::unique_ptr<class VulkanDevice> device;
	std::unique_ptr<class VulkanSwapchain> swapchain;
	
	VkCommandPool commandPool;
	std::vector<VkCommandBuffer> commandBuffers;
	
	void initInstance(SDL_Window* window);
	void initCommandPool();
	void initCommandBuffers();
	void initSynchronization();

	std::vector<const char*> requiredInstanceExtensions(SDL_Window* window);
};
