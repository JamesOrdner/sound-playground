#include "VulkanAllocator.h"
#define VMA_IMPLEMENTATION
#include <vma/vk_mem_alloc.h>
#include <utility>

VulkanBuffer::VulkanBuffer() :
    buffer(VK_NULL_HANDLE),
    allocation(VK_NULL_HANDLE)
{
}

VulkanBuffer::VulkanBuffer(VulkanBuffer&& other) noexcept :
    buffer(std::exchange(other.buffer, nullptr)),
    allocation(std::exchange(other.allocation, nullptr))
{
}

VulkanBuffer& VulkanBuffer::operator=(VulkanBuffer&& other) noexcept
{
    buffer = std::exchange(other.buffer, nullptr);
    allocation = std::exchange(other.allocation, nullptr);
    return *this;
}

VulkanImage::VulkanImage() :
    image(VK_NULL_HANDLE),
    allocation(VK_NULL_HANDLE)
{
}

VulkanImage::VulkanImage(VulkanImage&& other) noexcept :
    image(std::exchange(other.image, nullptr)),
    allocation(std::exchange(other.allocation, nullptr))
{
}

VulkanImage& VulkanImage::operator=(VulkanImage&& other) noexcept
{
    image = std::exchange(other.image, nullptr);
    allocation = std::exchange(other.allocation, nullptr);
    return *this;
}

VulkanAllocator::VulkanAllocator(const VkDevice device, const VkPhysicalDevice physicalDevice)
{
    VmaAllocatorCreateInfo createInfo{
        .physicalDevice = physicalDevice,
        .device = device
    };
    
    vmaCreateAllocator(&createInfo, &allocator);
}

VulkanAllocator::~VulkanAllocator()
{
    vmaDestroyAllocator(allocator);
}

VulkanBuffer VulkanAllocator::createBuffer(const VkBufferCreateInfo& createInfo, const VmaAllocationCreateInfo& allocInfo) const
{
    VulkanBuffer vb;
    if (vmaCreateBuffer(allocator, &createInfo, &allocInfo, &vb.buffer, &vb.allocation, nullptr) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create Vulkan buffer!");
    }
    
    return vb;
}

void VulkanAllocator::destroyBuffer(const VulkanBuffer& buffer) const
{
    vmaDestroyBuffer(allocator, buffer.buffer, buffer.allocation);
}

VulkanImage VulkanAllocator::createImage(const VkImageCreateInfo& createInfo, VkMemoryPropertyFlags properties) const
{
    VmaAllocationCreateInfo allocInfo{
        .usage = VMA_MEMORY_USAGE_GPU_ONLY,
        .requiredFlags = properties
    };
    
    VulkanImage vi;
    if (vmaCreateImage(allocator, &createInfo, &allocInfo, &vi.image, &vi.allocation, nullptr) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create Vulkan image!");
    }
    
    return vi;
}

void VulkanAllocator::destroyImage(const VulkanImage& image) const
{
    vmaDestroyImage(allocator, image.image, image.allocation);
}

void VulkanAllocator::map(const VulkanBuffer& buffer, void** data) const
{
    vmaMapMemory(allocator, buffer.allocation, data);
}

void VulkanAllocator::unmap(const VulkanBuffer& buffer) const
{
    vmaUnmapMemory(allocator, buffer.allocation);
}

void VulkanAllocator::map(const VulkanImage& image, void** data) const
{
    vmaMapMemory(allocator, image.allocation, data);
}

void VulkanAllocator::unmap(const VulkanImage& image) const
{
    vmaUnmapMemory(allocator, image.allocation);
}

void VulkanAllocator::flush(const VulkanBuffer& buffer, VkDeviceSize offset, VkDeviceSize size) const
{
	vmaFlushAllocation(allocator, buffer.allocation, offset, size);
}

void VulkanAllocator::flush(const VulkanImage& image, VkDeviceSize offset, VkDeviceSize size) const
{
	vmaFlushAllocation(allocator, image.allocation, offset, size);
}
