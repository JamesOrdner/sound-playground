#pragma once

#include <SDL_video.h>
#include <vulkan/vulkan.h>
#include <memory>
#include <vector>
#include <array>
#include <map>
#include <string>

class VulkanInstance
{
public:
	
	VulkanInstance(SDL_Window* window);
	
	~VulkanInstance();

	class VulkanScene* createScene();
	void destroyScene(class VulkanScene* scene);
	
	class VulkanUI* createUI();
	void destroyUI(class VulkanUI* ui);
	
	/// Return a shared pointer to the specified mesh, creating the mesh if not yet loaded
	class VulkanMesh* sharedMesh(const std::string& filepath);
	
	/// Return a shared pointer to the specified material, creating the material if not yet loaded
	class VulkanMaterial* sharedMaterial(const std::string& name);

	/// Begin rendering a frame
	void beginFrame();

	/// During frame rendering, render a scene and (optionally) a UI to the framebuffer
	void draw(class VulkanScene* scene, class VulkanUI* ui = nullptr);

	/// End frame rendering and present to the swapchain
	void endFrameAndPresent();
	
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
	std::vector<std::unique_ptr<class VulkanUI>> uis;
	
	/// Maps mesh filepaths to the corresponding mesh objects
	std::map<std::string, std::unique_ptr<class VulkanMesh>> meshes;
	
	/// Maps material names to the corresponding material objects
	std::map<std::string, std::unique_ptr<class VulkanMaterial>> materials;
	
	std::unique_ptr<class VulkanShadow> shadow;
	
	void initInstance(SDL_Window* window);
	void initRenderPass();
	void initCommandPool();
	
	std::vector<const char*> requiredInstanceExtensions(SDL_Window* window);
};
