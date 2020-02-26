#include "VulkanSwapchain.h"
#include "VulkanDevice.h"

VulkanSwapchain::VulkanSwapchain(const VulkanDevice* device, VkSurfaceKHR surface) :
	device(device)
{
	initSwapchain(surface);
	initImageViews();
	initDepthImage();
	initRenderPass();
	initFramebuffers();
}

VulkanSwapchain::~VulkanSwapchain()
{
	for (size_t i = 0; i < imageViews.size(); i++) {
		vkDestroyFramebuffer(device->vkDevice(), framebuffers[i], nullptr);
		vkDestroyImageView(device->vkDevice(), imageViews[i], nullptr);
	}
	framebuffers.clear();
	imageViews.clear();
	
	vkDestroyRenderPass(device->vkDevice(), renderPass, nullptr);
	
	vkDestroyImageView(device->vkDevice(), depthImageView, nullptr);
	device->allocator()->destroyImage(depthImage);
	
	vkDestroySwapchainKHR(device->vkDevice(), swapchain, nullptr);
}

void VulkanSwapchain::initSwapchain(VkSurfaceKHR surface)
{
	auto capabilities = device->surfaceCapabilities(surface);
	auto formats = device->surfaceFormats(surface);
	auto presentModes = device->surfacePresentModes(surface);
	
	extent = capabilities.currentExtent;
	imageFormat = formats[0];
	
	// search for preferred format
	for (const auto& format : formats) {
		if (format.format == VK_FORMAT_B8G8R8A8_UNORM && format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
			imageFormat = format;
			break;
		}
	}
	
	VkSwapchainCreateInfoKHR swapchainInfo{
		.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
		.surface = surface,
		.minImageCount = capabilities.minImageCount + 1,
		.imageFormat = imageFormat.format,
		.imageColorSpace = imageFormat.colorSpace,
		.imageExtent = extent,
		.imageArrayLayers = 1,
		.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
		.preTransform = capabilities.currentTransform,
		.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
		.presentMode = VK_PRESENT_MODE_FIFO_KHR,
		.clipped = VK_TRUE
	};
	
	if (capabilities.maxImageCount && (swapchainInfo.minImageCount > capabilities.maxImageCount)) {
		swapchainInfo.minImageCount = capabilities.maxImageCount;
	}
	
	if (vkCreateSwapchainKHR(device->vkDevice(), &swapchainInfo, nullptr, &swapchain) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create Vulkan swapchain!");
	}
}

void VulkanSwapchain::initImageViews()
{
	uint32_t imageCount;
	vkGetSwapchainImagesKHR(device->vkDevice(), swapchain, &imageCount, nullptr);
	std::vector<VkImage> images(imageCount);
	vkGetSwapchainImagesKHR(device->vkDevice(), swapchain, &imageCount, images.data());
	
	imageViews.resize(imageCount);
	for (size_t i = 0; i < imageCount; i++) {
		VkImageViewCreateInfo imageViewInfo{
			.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
			.image = images[i],
			.viewType = VK_IMAGE_VIEW_TYPE_2D,
			.format = imageFormat.format,
			.components = {
				VK_COMPONENT_SWIZZLE_R,
				VK_COMPONENT_SWIZZLE_G,
				VK_COMPONENT_SWIZZLE_B,
				VK_COMPONENT_SWIZZLE_A
			},
			.subresourceRange = {
				.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
				.baseMipLevel = 0,
				.levelCount = 1,
				.baseArrayLayer = 0,
				.layerCount = 1
			}
		};
		
		if (vkCreateImageView(device->vkDevice(), &imageViewInfo, nullptr, &imageViews[i]) != VK_SUCCESS) {
			throw std::runtime_error("Failed to create Vulkan swapchain image views!");
		}
	}
}

void VulkanSwapchain::initDepthImage()
{
	depthImageFormat = device->firstSupportedFormat(
		{ VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
		VK_IMAGE_TILING_OPTIMAL,
		VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
	);

	if (depthImageFormat == VK_FORMAT_UNDEFINED) {
		throw std::runtime_error("No supported Vulkan image formats for depth attachment!");
	}
    
    VkImageCreateInfo imageInfo{
        .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
        .imageType = VK_IMAGE_TYPE_2D,
        .format = depthImageFormat,
        .extent = { extent.width, extent.width, 1 },
        .mipLevels = 1,
        .arrayLayers = 1,
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .tiling = VK_IMAGE_TILING_OPTIMAL,
        .usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED
    };
    VkMemoryPropertyFlags properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    depthImage = device->allocator()->createImage(imageInfo, properties);

	VkImageViewCreateInfo imageViewInfo{
		.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
		.image = depthImage.image,
		.viewType = VK_IMAGE_VIEW_TYPE_2D,
		.format = depthImageFormat,
		.subresourceRange = {
			.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT,
			.baseMipLevel = 0,
			.levelCount = 1,
			.baseArrayLayer = 0,
			.layerCount = 1
		}
	};

	if (vkCreateImageView(device->vkDevice(), &imageViewInfo, nullptr, &depthImageView) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create Vulkan depth image view!");
	}
}

void VulkanSwapchain::initRenderPass()
{
	VkAttachmentDescription colorAttachmentDesc{
		.format = imageFormat.format,
		.samples = VK_SAMPLE_COUNT_1_BIT,
		.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
		.storeOp = VK_ATTACHMENT_STORE_OP_STORE,
		.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
		.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
		.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
		.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
	};
	
	VkAttachmentDescription depthAttachmentDesc{
		.format = depthImageFormat,
		.samples = VK_SAMPLE_COUNT_1_BIT,
		.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
		.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
		.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
		.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
		.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
		.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
	};
	
	VkAttachmentReference colorAttachmentRef{
		.attachment = 0,
		.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
	};
	
	VkAttachmentReference depthAttachmentRef{
		.attachment = 1,
		.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
	};
	
	VkSubpassDescription subpassDesc{
		.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
		.colorAttachmentCount = 1,
		.pColorAttachments = &colorAttachmentRef,
		.pDepthStencilAttachment = &depthAttachmentRef
	};
	
	VkAttachmentDescription attachments[] = { colorAttachmentDesc, depthAttachmentDesc };
	VkRenderPassCreateInfo renderPassInfo{
		.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
		.attachmentCount = 2,
		.pAttachments = attachments,
		.subpassCount = 1,
		.pSubpasses = &subpassDesc
	};
	
	if (vkCreateRenderPass(device->vkDevice(), &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create Vulkan render pass!");
	}
}

void VulkanSwapchain::initFramebuffers()
{
	framebuffers.resize(imageViews.size());
	for (size_t i = 0; i < framebuffers.size(); i++) {
		VkImageView attachments[] = { imageViews[i], depthImageView };
		VkFramebufferCreateInfo framebufferInfo{
			.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
			.renderPass = renderPass,
			.attachmentCount = 2,
			.pAttachments = attachments,
			.width = extent.width,
			.height = extent.height,
			.layers = 1,
		};
		
		if (vkCreateFramebuffer(device->vkDevice(), &framebufferInfo, nullptr, &framebuffers[i]) != VK_SUCCESS) {
			throw std::runtime_error("Failed to create Vulkan swapchain framebuffers!");
		}
	}
}
