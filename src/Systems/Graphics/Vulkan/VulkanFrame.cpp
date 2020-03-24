#include "VulkanFrame.h"
#include "VulkanDevice.h"
#include "VulkanScene.h"
#include "VulkanUI.h"
#include "VulkanMaterial.h"
#include "VulkanShadow.h"
#include "VulkanModel.h"
#include "VulkanMesh.h"
#include <stdexcept>

constexpr size_t maxModelCount = 128;

struct MainUBO {
	mat::mat4 modelViewMatrix;
	mat::mat4 shadowMatrix;
};

VulkanFrame::VulkanFrame(const VulkanDevice* device, VkCommandPool commandPool) :
	device(device)
{
	// Command buffer
	
	VkCommandBufferAllocateInfo commandBufferInfo{
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
		.commandPool = commandPool,
		.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
		.commandBufferCount = 1
	};
	
	if (vkAllocateCommandBuffers(device->vkDevice(), &commandBufferInfo, &commandBuffer) != VK_SUCCESS) {
		throw std::runtime_error("Failed to allocate Vulkan command buffers!");
	}
	
	// Synchronization
	
	VkFenceCreateInfo fenceInfo{
		.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
		.flags = VK_FENCE_CREATE_SIGNALED_BIT
	};
	
	if (vkCreateFence(device->vkDevice(), &fenceInfo, nullptr, &completeFence) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create Vulkan frame fence!");
	}
	
	VkSemaphoreCreateInfo semaphoreInfo{
		.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO
	};
	
	if (vkCreateSemaphore(device->vkDevice(), &semaphoreInfo, nullptr, &completeSemaphore) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create Vulkan frame semaphore!");
	}
	
	// DescriptorPool
	
	std::array<VkDescriptorPoolSize, 3> descriptorPoolSizes{
		VkDescriptorPoolSize{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 2 },
		VkDescriptorPoolSize{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1 },
		VkDescriptorPoolSize{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1 }
	};
	
	VkDescriptorPoolCreateInfo descriptorPoolInfo{
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
		.maxSets = 2, // (one material + shadow pipeline) * one scene
		.poolSizeCount = static_cast<uint32_t>(descriptorPoolSizes.size()),
		.pPoolSizes = descriptorPoolSizes.data()
	};
	
	if (vkCreateDescriptorPool(device->vkDevice(), &descriptorPoolInfo, nullptr, &descriptorPool) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create Vulkan descriptor pool!");
	}
	
	// Dynamic UBO alignment
	
	VkPhysicalDeviceLimits limits = device->physicalDeviceProperties().limits;
    size_t minUboAlignment = limits.minUniformBufferOffsetAlignment;
	uboAlignment = sizeof(MainUBO);
    if (minUboAlignment > 0) {
        uboAlignment = (uboAlignment + minUboAlignment - 1) & ~(minUboAlignment - 1);
    }
}

VulkanFrame::~VulkanFrame()
{
	vkDestroyDescriptorPool(device->vkDevice(), descriptorPool, nullptr);
	vkDestroySemaphore(device->vkDevice(), completeSemaphore, nullptr);
	vkDestroyFence(device->vkDevice(), completeFence, nullptr);
	
	for (auto& pair : sceneData) pair.second.deinit(device->allocator());
	for (auto& pair : uiData)    pair.second.deinit(device->allocator());
}

void VulkanFrame::registerScene(const VulkanScene* scene)
{
	SceneData& data = sceneData[scene];
	data.init(device->allocator(), maxModelCount * uboAlignment);
}

void VulkanFrame::unregisterScene(const VulkanScene* scene)
{
	sceneData[scene].deinit(device->allocator());
	sceneData.erase(scene);
}

void VulkanFrame::SceneData::init(const VulkanAllocator& allocator, VkDeviceSize transformsBufferSize)
{
	VkBufferCreateInfo bufferInfo{
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size = transformsBufferSize,
        .usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT
    };
	
    VmaAllocationCreateInfo bufferAllocInfo{
        .usage = VMA_MEMORY_USAGE_CPU_TO_GPU,
        .requiredFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_CACHED_BIT
    };
	
	modelTransforms = allocator.createBuffer(bufferInfo, bufferAllocInfo);
	allocator.map(modelTransforms, &modelTransformsData);
	
	modelShadowTransforms = allocator.createBuffer(bufferInfo, bufferAllocInfo);
	allocator.map(modelShadowTransforms, &modelShadowTransformsData);
	
	VkBufferCreateInfo constantsBufferInfo{
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size = static_cast<uint32_t>(sizeof(mat::mat4)),
        .usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT
    };
	
    VmaAllocationCreateInfo constantsBufferAllocInfo{
        .usage = VMA_MEMORY_USAGE_CPU_TO_GPU,
        .requiredFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
    };
	
	constants = allocator.createBuffer(constantsBufferInfo, constantsBufferAllocInfo);
	allocator.map(constants, &constantsData);
}

void VulkanFrame::SceneData::deinit(const VulkanAllocator& allocator)
{
	allocator.unmap(constants);
	allocator.unmap(modelShadowTransforms);
	allocator.unmap(modelTransforms);
	allocator.destroyBuffer(constants);
	allocator.destroyBuffer(modelShadowTransforms);
	allocator.destroyBuffer(modelTransforms);
}

void VulkanFrame::registerUI(const VulkanUI* ui)
{
	UIData& data = uiData[ui];
	data.init();
}

void VulkanFrame::unregisterUI(const VulkanUI* ui)
{
	uiData[ui].deinit(device->allocator());
	uiData.erase(ui);
}

void VulkanFrame::UIData::init()
{
	vertexBuffer.buffer = VK_NULL_HANDLE;
	bufferData = nullptr;
	bufferCapacity = 0;
}

void VulkanFrame::UIData::deinit(const VulkanAllocator& allocator)
{
	if (vertexBuffer.buffer) {
		allocator.unmap(vertexBuffer);
		allocator.destroyBuffer(vertexBuffer);
	}
}

void VulkanFrame::updateSceneData(const VulkanScene* scene)
{
	SceneData& data = sceneData[scene];
	
	for (const auto& model : scene->models) {
		uint32_t offset = model->modelID * static_cast<uint32_t>(uboAlignment);
		char* dest;
		
		// shadow
		mat::mat4 lightView = mat::lookAt(mat::vec3{ 0.3f, 1.f, 0.1f }, mat::vec3());
		mat::mat4 lightMVP = mat::t(mat::ortho(-6, 6, -6, 6, -10, 10) * lightView * model->transform);
		dest = static_cast<char*>(data.modelShadowTransformsData) + offset;
		std::copy_n(&lightMVP, 1, reinterpret_cast<mat::mat4*>(dest));
		
		// main
		MainUBO mainUBO;
		mainUBO.modelViewMatrix = mat::t(scene->getViewMatrix() * model->transform);
		mainUBO.shadowMatrix = lightMVP;
		dest = static_cast<char*>(data.modelTransformsData) + offset;
		std::copy_n(&mainUBO, 1, reinterpret_cast<MainUBO*>(dest));
	}
	
	mat::mat4 transposed = mat::t(scene->getProjMatrix());
	std::copy_n(&transposed, 1, reinterpret_cast<mat::mat4*>(data.constantsData));
	
	device->allocator().flush(data.modelTransforms);
	device->allocator().flush(data.modelShadowTransforms);
}

void VulkanFrame::updateUIData(const VulkanUI* ui)
{
	std::vector<mat::vec2> buffer;
	buffer.reserve(ui->objects.size() * 12); // 6 verts * 2 attr per object
	
	for (const auto& object : ui->objects) {
		buffer.push_back(object->position);
		buffer.push_back(object->uv_position);
		buffer.push_back(object->position + object->bounds);
		buffer.push_back(object->uv_position + object->uv_bounds);
		buffer.push_back(mat::vec2{ object->position.x + object->bounds.x, object->position.y });
		buffer.push_back(mat::vec2{ object->uv_position.x + object->uv_bounds.x, object->uv_position.y });
		
		buffer.push_back(object->position);
		buffer.push_back(object->uv_position);
		buffer.push_back(mat::vec2{ object->position.x, object->position.y + object->bounds.y });
		buffer.push_back(mat::vec2{ object->uv_position.x, object->uv_position.y + object->uv_bounds.y });
		buffer.push_back(object->position + object->bounds);
		buffer.push_back(object->uv_position + object->uv_bounds);
	}
	
	UIData& data = uiData[ui];
	
	VkDeviceSize requiredSize = buffer.size() * sizeof(mat::vec2);
	if (data.bufferCapacity < requiredSize) {
		data.growCapacity(device->allocator(), requiredSize);
	}
	
	std::copy(buffer.cbegin(), buffer.cend(), static_cast<mat::vec2*>(data.bufferData));
}

void VulkanFrame::UIData::growCapacity(const VulkanAllocator& allocator, VkDeviceSize newCapacity)
{
	deinit(allocator);
	
	VkBufferCreateInfo bufferInfo{
		.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
		.size = newCapacity,
		.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT
	};
	
	VmaAllocationCreateInfo allocInfo{
		.usage = VMA_MEMORY_USAGE_CPU_TO_GPU,
		.requiredFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
	};
	
	vertexBuffer = allocator.createBuffer(bufferInfo, allocInfo);
	allocator.map(vertexBuffer, &bufferData);
	bufferCapacity = newCapacity;
}

void VulkanFrame::updateDescriptorSets(const std::vector<VulkanMaterial*>& materials, const VulkanShadow* shadow)
{
	vkResetDescriptorPool(device->vkDevice(), descriptorPool, 0);
	descriptorSets.clear();
	
	// TODO: we're assuming only one scene
	SceneData& data = sceneData.begin()->second;
	
	for (const auto* material : materials) {
		VkDescriptorSetAllocateInfo descriptorSetAllocInfo{
			.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
			.descriptorPool = descriptorPool,
			.descriptorSetCount = 1,
			.pSetLayouts = &material->descriptorSetLayout
		};
		
		VkDescriptorSet descriptorSet;
		if (vkAllocateDescriptorSets(device->vkDevice(), &descriptorSetAllocInfo, &descriptorSet) != VK_SUCCESS) {
			throw std::runtime_error("failed to allocate descriptor sets!");
		}
		
		descriptorSets[material->name] = descriptorSet;
		
		VkDescriptorBufferInfo modelTransformDescriptorBufferInfo{
			.buffer = data.modelTransforms.buffer,
			.offset = 0,
			.range = uboAlignment
		};
		
		VkWriteDescriptorSet modelTransformDescriptorWrite{
			.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
			.dstSet = descriptorSet,
			.dstBinding = 0,
			.descriptorCount = 1,
			.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,
			.pBufferInfo = &modelTransformDescriptorBufferInfo
		};
		
		VkDescriptorBufferInfo constantsDescriptorBufferInfo{
			.buffer = data.constants.buffer,
			.offset = 0,
			.range = sizeof(mat::mat4)
		};
		
		VkWriteDescriptorSet constantsDescriptorWrite{
			.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
			.dstSet = descriptorSet,
			.dstBinding = 1,
			.descriptorCount = 1,
			.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
			.pBufferInfo = &constantsDescriptorBufferInfo
		};
		
		VkDescriptorImageInfo shadowSamplerDescriptorImageInfo{
			.sampler = shadow->sampler,
			.imageView = shadow->imageView,
			.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL
		};
		
		VkWriteDescriptorSet shadowSamplerDescriptorWrite{
			.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
			.dstSet = descriptorSet,
			.dstBinding = 2,
			.descriptorCount = 1,
			.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
			.pImageInfo = &shadowSamplerDescriptorImageInfo
		};
		
		std::array<VkWriteDescriptorSet, 3> writeDescriptorSets{
			modelTransformDescriptorWrite,
			constantsDescriptorWrite,
			shadowSamplerDescriptorWrite
		};
		
		vkUpdateDescriptorSets(device->vkDevice(), writeDescriptorSets.size(), writeDescriptorSets.data(), 0, nullptr);
	}
	
	// shadow
	
	VkDescriptorSetAllocateInfo descriptorSetAllocInfo{
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
		.descriptorPool = descriptorPool,
		.descriptorSetCount = 1,
		.pSetLayouts = &shadow->descriptorSetLayout
	};
	
	VkDescriptorSet descriptorSet;
	if (vkAllocateDescriptorSets(device->vkDevice(), &descriptorSetAllocInfo, &descriptorSet) != VK_SUCCESS) {
		throw std::runtime_error("failed to allocate descriptor sets!");
	}
	
	descriptorSets["shadow"] = descriptorSet;
	
	VkDescriptorBufferInfo descriptorBufferInfo{
		.buffer = data.modelShadowTransforms.buffer,
		.offset = 0,
		.range = uboAlignment
	};
	
	VkWriteDescriptorSet descriptorWrite{
		.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
		.dstSet = descriptorSet,
		.dstBinding = 0,
		.dstArrayElement = 0,
		.descriptorCount = 1,
		.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,
		.pBufferInfo = &descriptorBufferInfo
	};
	
	vkUpdateDescriptorSets(device->vkDevice(), 1, &descriptorWrite, 0, nullptr);
}

void VulkanFrame::beginFrame()
{
	vkWaitForFences(device->vkDevice(), 1, &completeFence, VK_TRUE, UINT64_MAX);
	vkResetFences(device->vkDevice(), 1, &completeFence);
	
	VkCommandBufferBeginInfo commandBufferBeginInfo{
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
		.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT
	};
	
	vkBeginCommandBuffer(commandBuffer, &commandBufferBeginInfo);
}

void VulkanFrame::render(
	const VulkanScene* scene,
	const VulkanUI* ui,
	VulkanShadow* shadow,
	VkRenderPass sceneRenderPass,
	VkFramebuffer framebuffer,
	const VkRect2D& renderArea)
{
	renderShadowPass(scene, shadow);
	
	// render scene and UI
	
	VkClearValue clearValues[] = {
		{.color = {.float32 = { 0.0f, 0.0f, 0.0f, 1.0f }}},
		{.depthStencil = { 1.f, 0 }}
	};
	
	VkRenderPassBeginInfo renderPassBeginInfo{
		.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
		.renderPass = sceneRenderPass,
		.framebuffer = framebuffer,
		.renderArea = renderArea,
		.clearValueCount = 2,
		.pClearValues = clearValues
	};
	
	vkCmdBeginRenderPass(commandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
	
	renderScene(scene);
	if (ui) renderUI(ui);
	
	vkCmdEndRenderPass(commandBuffer);
}

void VulkanFrame::renderShadowPass(const VulkanScene* scene, VulkanShadow* shadow)
{
	VkClearValue clearValue{
		.depthStencil = { 1.f, 0 }
	};
	
	VkRenderPassBeginInfo renderPassBeginInfo{
		.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
		.renderPass = shadow->renderPass,
		.framebuffer = shadow->framebuffer,
		.renderArea = shadow->renderArea,
		.clearValueCount = 1,
		.pClearValues = &clearValue
	};
	
	vkCmdBeginRenderPass(commandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, shadow->pipeline);
	
	VkDescriptorSet shadowDescriptorSet = descriptorSets["shadow"];
	VkDeviceSize offsets[] = { 0 };
	for (const auto& model : scene->models) {
		const VulkanMesh* mesh = model->getMesh();
		uint32_t dynamicOffset = model->modelID * static_cast<uint32_t>(uboAlignment);
		vkCmdBindVertexBuffers(commandBuffer, 0, 1, &mesh->vertexBuffer.buffer, offsets);
		vkCmdBindIndexBuffer(commandBuffer, mesh->vertexBuffer.buffer, mesh->indexBufferOffset, VK_INDEX_TYPE_UINT16);
		vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, shadow->pipelineLayout, 0, 1, &shadowDescriptorSet, 1, &dynamicOffset);
		vkCmdDrawIndexed(commandBuffer, mesh->indexBuffer.size(), 1, 0, 0, 0);
	}
	
	vkCmdEndRenderPass(commandBuffer);
}

void VulkanFrame::renderScene(const VulkanScene* scene)
{
	const VulkanMaterial* material = nullptr;
	const VulkanMesh* mesh = nullptr;
	VkDescriptorSet descriptorSet;
	VkDeviceSize offsets[] = { 0 };
	for (const auto& model : scene->models) {
		if (material != model->getMaterial()) {
			material = model->getMaterial();
			if (material) {
				material->bind(commandBuffer);
				descriptorSet = descriptorSets.find(material->name)->second;
			}
		}
		
		if (mesh != model->getMesh()) {
			mesh = model->getMesh();
			if (mesh) {
				vkCmdBindVertexBuffers(commandBuffer, 0, 1, &mesh->vertexBuffer.buffer, offsets);
				vkCmdBindIndexBuffer(commandBuffer, mesh->vertexBuffer.buffer, mesh->indexBufferOffset, VK_INDEX_TYPE_UINT16);
			}
		}
		
		if (!material || !mesh) break;
		
		uint32_t dynamicOffset = model->modelID * static_cast<uint32_t>(uboAlignment);
		vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, material->pipelineLayout, 0, 1, &descriptorSet, 1, &dynamicOffset);
		vkCmdDrawIndexed(commandBuffer, mesh->indexBuffer.size(), 1, 0, 0, 0);
	}
}

void VulkanFrame::renderUI(const VulkanUI* ui)
{
	const UIData& data = uiData[ui];
	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, ui->pipeline);
	VkDeviceSize offsets[] = { 0 };
	vkCmdBindVertexBuffers(commandBuffer, 0, 1, &data.vertexBuffer.buffer, offsets);
	vkCmdDraw(commandBuffer, ui->objects.size() * 6, 1, 0, 0);
}

VkSemaphore VulkanFrame::endFrame(VkSemaphore acquireSemaphore)
{
	vkEndCommandBuffer(commandBuffer);
	
	VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT };
	VkSubmitInfo submitInfo{
		.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
		.waitSemaphoreCount = 1,
		.pWaitSemaphores = &acquireSemaphore,
		.pWaitDstStageMask = waitStages,
		.commandBufferCount = 1,
		.pCommandBuffers = &commandBuffer,
		.signalSemaphoreCount = 1,
		.pSignalSemaphores = &completeSemaphore
	};
	
	vkQueueSubmit(device->queues().graphics.queue, 1, &submitInfo, completeFence);
	
	return completeSemaphore;
}
