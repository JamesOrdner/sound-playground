#pragma once

#include <vma/vk_mem_alloc.h>

struct VulkanBuffer
{
    VulkanBuffer();
    
    VulkanBuffer(const VulkanBuffer&) = delete;
    VulkanBuffer& operator=(const VulkanBuffer&) = delete;
    
    VulkanBuffer(VulkanBuffer&& other) noexcept;
    VulkanBuffer& operator=(VulkanBuffer&& other) noexcept;
    
    VkBuffer buffer;
    VmaAllocation allocation;
};

struct VulkanImage
{
    VulkanImage();
    
    VulkanImage(const VulkanImage&) = delete;
    VulkanImage& operator=(const VulkanImage&) = delete;
    
    VulkanImage(VulkanImage&& other) noexcept;
    VulkanImage& operator=(VulkanImage&& other) noexcept;
    
    VkImage image;
    VmaAllocation allocation;
};

class VulkanAllocator
{
public:
    
    VulkanAllocator(const VkDevice device, const VkPhysicalDevice physicalDevice);
    
    ~VulkanAllocator();
    
    /// Creates a buffer, allocating and binding the necessary memory
    VulkanBuffer createBuffer(const VkBufferCreateInfo& createInfo, const VmaAllocationCreateInfo& allocInfo) const;
    void destroyBuffer(const VulkanBuffer& buffer) const;
    
    /// Creates an image, allocating and binding the necessary memory
    VulkanImage createImage(const VkImageCreateInfo& createInfo, VkMemoryPropertyFlags properties) const;
    void destroyImage(const VulkanImage& image) const;
    
    void map(const VulkanBuffer& buffer, void** data) const;
    void unmap(const VulkanBuffer& buffer) const;
	
	void map(const VulkanImage& image, void** data) const;
    void unmap(const VulkanImage& image) const;
	
	void flush(const VulkanBuffer& buffer, VkDeviceSize offset = 0, VkDeviceSize size = VK_WHOLE_SIZE) const;
	void flush(const VulkanImage& image, VkDeviceSize offset = 0, VkDeviceSize size = VK_WHOLE_SIZE) const;
    
private:
    
    VmaAllocator allocator;
};
