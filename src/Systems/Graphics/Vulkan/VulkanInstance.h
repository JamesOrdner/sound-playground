#pragma once

#include <SDL_video.h>
#include <vulkan/vulkan.h>
#include <memory>
#include <vector>
#include <array>

class VulkanInstance
{
public:
	
	VulkanInstance(SDL_Window* window);
	
	~VulkanInstance();
	
	void renderFrame();
	
	inline class VulkanDevice* getDevice() const { return device.get(); }
	
private:
	
	VkInstance instance;
	VkSurfaceKHR surface;
	VkRenderPass renderPass;

	std::unique_ptr<class VulkanDevice> device;
	std::unique_ptr<class VulkanSwapchain> swapchain;
	
	VkCommandPool commandPool;
	std::array<std::unique_ptr<class VulkanFrame>, 2> frames;
	uint32_t frameIndex;
	
	/// This array is sorted so that all models sharing a mesh are contiguous
	std::vector<std::unique_ptr<class VulkanModel>> models;
	
	void initInstance(SDL_Window* window);
	void initRenderPass();
	void initCommandPool();
	
	std::vector<const char*> requiredInstanceExtensions(SDL_Window* window);
};
