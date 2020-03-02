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

	class VulkanScene* createScene();
	void destroyScene(class VulkanScene* scene);

	/// Begin rendering a frame
	void beginRender();

	/// During active rendering, render a scene to the framebuffer
	void renderScene(class VulkanScene* scene);

	/// End frame rendering and present to the swapchain
	void endRenderAndPresent();
	
private:
	
	VkInstance instance;
	VkSurfaceKHR surface;
	VkRenderPass renderPass;
	
	std::unique_ptr<class VulkanDevice> device;
	std::unique_ptr<class VulkanSwapchain> swapchain;
	
	VkCommandPool commandPool;
	std::array<std::unique_ptr<class VulkanFrame>, 2> frames;
	uint32_t frameIndex;

	class VulkanFrame* activeFrame;
	uint32_t activeSwapchainImageIndex;
	VkSemaphore activeFrameAcquireSemaphore;

	std::vector<std::unique_ptr<class VulkanScene>> scenes;
	
	void initInstance(SDL_Window* window);
	void initRenderPass();
	void initCommandPool();
	
	std::vector<const char*> requiredInstanceExtensions(SDL_Window* window);
};
