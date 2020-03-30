#include "VulkanFrame.h"
#include "VulkanDevice.h"
#include "VulkanScene.h"
#include "VulkanTexture.h"
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

VulkanFrame::VulkanFrame(const VulkanDevice* device, const VulkanPipelineLayouts& layouts, const VulkanShadow& shadow, VkCommandPool commandPool) :
	device(device)
{
	// frame command buffer
	
	VkCommandBufferAllocateInfo commandBufferInfo{
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
		.commandPool = commandPool,
		.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
		.commandBufferCount = 1
	};
	
	if (vkAllocateCommandBuffers(device->vkDevice(), &commandBufferInfo, &commandBuffer) != VK_SUCCESS) {
		throw std::runtime_error("Failed to allocate Vulkan command buffers!");
	}
	
	// synchronization
	
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
	
	// descriptor pool for per-frame descriptors
	
	// TODO: get from VulkanPipelineLayouts?
	std::array<VkDescriptorPoolSize, 3> descriptorPoolSizes{
		VkDescriptorPoolSize{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 2 },
		VkDescriptorPoolSize{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1 },
		VkDescriptorPoolSize{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1 }
	};
	
	VkDescriptorPoolCreateInfo descriptorPoolInfo{
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
		.maxSets = 3, // TODO: get from VulkanPipelineLayouts
		.poolSizeCount = static_cast<uint32_t>(descriptorPoolSizes.size()),
		.pPoolSizes = descriptorPoolSizes.data()
	};
	
	if (vkCreateDescriptorPool(device->vkDevice(), &descriptorPoolInfo, nullptr, &descriptorPool) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create Vulkan descriptor pool!");
	}
	
	// dynamic UBO alignment
	
	VkPhysicalDeviceLimits limits = device->physicalDeviceProperties().limits;
    size_t minUboAlignment = limits.minUniformBufferOffsetAlignment;
	uboAlignment = sizeof(MainUBO);
    if (minUboAlignment > 0) {
        uboAlignment = (uboAlignment + minUboAlignment - 1) & ~(minUboAlignment - 1);
    }
	
	// init descriptor sets and write static descriptors
	
	initDescriptorSets(layouts, shadow);
}

VulkanFrame::~VulkanFrame()
{
	vkDestroyDescriptorPool(device->vkDevice(), descriptorPool, nullptr);
	vkDestroySemaphore(device->vkDevice(), completeSemaphore, nullptr);
	vkDestroyFence(device->vkDevice(), completeFence, nullptr);
	
	for (auto& pair : sceneData) pair.second.deinit(device->allocator());
	for (auto& pair : uiData)    pair.second.deinit(device->allocator());
}

void VulkanFrame::initDescriptorSets(const VulkanPipelineLayouts& layouts, const VulkanShadow& shadow)
{
	// object shader descriptor sets
	{
		const auto& objectLayout = layouts.getObjectLayout();
		
		VkDescriptorSetAllocateInfo descriptorSetAllocInfo{
			.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
			.descriptorPool = descriptorPool,
			.descriptorSetCount = static_cast<uint32_t>(objectLayout.descriptorSetLayouts.size()),
			.pSetLayouts = objectLayout.descriptorSetLayouts.data()
		};
		
		std::vector<VkDescriptorSet> newDescriptorSets(objectLayout.descriptorSetLayouts.size());
		if (vkAllocateDescriptorSets(device->vkDevice(), &descriptorSetAllocInfo, newDescriptorSets.data()) != VK_SUCCESS) {
			throw std::runtime_error("failed to allocate descriptor sets!");
		}
		
		descriptorSets[VulkanDescriptorSetType::ShadowSampler] = newDescriptorSets[0];
		descriptorSets[VulkanDescriptorSetType::ModelTransform] = newDescriptorSets[1];
	}
	
	// shadow map shader descriptor set
	{
		VkDescriptorSetAllocateInfo descriptorSetAllocInfo{
			.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
			.descriptorPool = descriptorPool,
			.descriptorSetCount = 1,
			.pSetLayouts = &shadow.descriptorSetLayout
		};
		
		VkDescriptorSet descriptorSet;
		if (vkAllocateDescriptorSets(device->vkDevice(), &descriptorSetAllocInfo, &descriptorSet) != VK_SUCCESS) {
			throw std::runtime_error("failed to allocate descriptor sets!");
		}
		
		descriptorSets[VulkanDescriptorSetType::ShadowTransform] = descriptorSet;
	}
	
	// write non-scene-dependent descriptor sets
	
	VkDescriptorImageInfo shadowSamplerDescriptorImageInfo{
		.sampler = shadow.sampler,
		.imageView = shadow.imageView,
		.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL
	};
	
	VkWriteDescriptorSet shadowSamplerDescriptorWrite{
		.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
		.dstSet = descriptorSets[VulkanDescriptorSetType::ShadowSampler],
		.dstBinding = 0,
		.descriptorCount = 1,
		.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
		.pImageInfo = &shadowSamplerDescriptorImageInfo
	};
	
	vkUpdateDescriptorSets(device->vkDevice(), 1, &shadowSamplerDescriptorWrite, 0, nullptr);
}

void VulkanFrame::registerScene(const VulkanScene* scene)
{
	SceneData& data = sceneData[scene];
	data.init(device->allocator(), maxModelCount * uboAlignment);
	
	VkDescriptorBufferInfo modelTransformBufferInfo{
		.buffer = data.modelTransforms.buffer,
		.offset = 0,
		.range = uboAlignment
	};
	
	VkWriteDescriptorSet modelTransformDescriptorWrite{
		.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
		.dstSet = descriptorSets[VulkanDescriptorSetType::ModelTransform],
		.dstBinding = 0,
		.descriptorCount = 1,
		.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,
		.pBufferInfo = &modelTransformBufferInfo
	};
	
	VkDescriptorBufferInfo shadowTransformBufferInfo{
		.buffer = data.modelShadowTransforms.buffer,
		.offset = 0,
		.range = uboAlignment
	};
	
	VkWriteDescriptorSet shadowTransformDescriptorWrite{
		.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
		.dstSet = descriptorSets[VulkanDescriptorSetType::ShadowTransform],
		.dstBinding = 0,
		.dstArrayElement = 0,
		.descriptorCount = 1,
		.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,
		.pBufferInfo = &shadowTransformBufferInfo
	};
	
	std::array<VkWriteDescriptorSet, 2> writeDescriptorSets{
		modelTransformDescriptorWrite,
		shadowTransformDescriptorWrite
	};
	
	vkUpdateDescriptorSets(device->vkDevice(), writeDescriptorSets.size(), writeDescriptorSets.data(), 0, nullptr);
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
}

void VulkanFrame::unregisterScene(const VulkanScene* scene)
{
	sceneData[scene].deinit(device->allocator());
	sceneData.erase(scene);
}

void VulkanFrame::SceneData::deinit(const VulkanAllocator& allocator)
{
	allocator.unmap(modelShadowTransforms);
	allocator.unmap(modelTransforms);
	allocator.destroyBuffer(modelShadowTransforms);
	allocator.destroyBuffer(modelTransforms);
}

void VulkanFrame::registerUI(const VulkanUI* ui)
{
	UIData& data = uiData[ui];
	data.init();
}

void VulkanFrame::UIData::init()
{
	vertexBuffer.buffer = VK_NULL_HANDLE;
	bufferData = nullptr;
	bufferCapacity = 0;
}

void VulkanFrame::unregisterUI(const VulkanUI* ui)
{
	uiData[ui].deinit(device->allocator());
	uiData.erase(ui);
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
	
	VkDescriptorSet shadowTransform = descriptorSets[VulkanDescriptorSetType::ShadowTransform];
	VkDeviceSize offsets[] = { 0 };
	for (const auto& model : scene->models) {
		const VulkanMesh* mesh = model->getMesh();
		uint32_t dynamicOffset = model->modelID * static_cast<uint32_t>(uboAlignment);
		vkCmdBindVertexBuffers(commandBuffer, 0, 1, &mesh->vertexBuffer.buffer, offsets);
		vkCmdBindIndexBuffer(commandBuffer, mesh->vertexBuffer.buffer, mesh->indexBufferOffset, VK_INDEX_TYPE_UINT16);
		vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, shadow->pipelineLayout, 0, 1, &shadowTransform, 1, &dynamicOffset);
		vkCmdDrawIndexed(commandBuffer, mesh->indexBuffer.size(), 1, 0, 0, 0);
	}
	
	vkCmdEndRenderPass(commandBuffer);
}

void VulkanFrame::renderScene(const VulkanScene* scene)
{
	VkDescriptorSet shadowSampler = descriptorSets[VulkanDescriptorSetType::ShadowSampler];
	VkDescriptorSet modelTransform = descriptorSets[VulkanDescriptorSetType::ModelTransform];
	VkDeviceSize offsets[] = { 0 };
	
	const VulkanMaterial* material = nullptr;
	const VulkanMesh* mesh = nullptr;
	for (const auto& model : scene->models) {
		if (material != model->getMaterial()) {
			material = model->getMaterial();
			if (material) {
				mat::mat4 pushData = mat::t(scene->getProjMatrix());
				vkCmdPushConstants(commandBuffer, material->layout.pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(mat::mat4), &pushData);
				material->bind(commandBuffer);
				vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, material->layout.pipelineLayout, 0, 1, &shadowSampler, 0, nullptr);
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
		vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, material->layout.pipelineLayout, 1, 1, &modelTransform, 1, &dynamicOffset);
		vkCmdDrawIndexed(commandBuffer, mesh->indexBuffer.size(), 1, 0, 0, 0);
	}
}

void VulkanFrame::renderUI(const VulkanUI* ui)
{
	const UIData& data = uiData[ui];
	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, ui->pipeline);
	VkDeviceSize offsets[] = { 0 };
	vkCmdBindVertexBuffers(commandBuffer, 0, 1, &data.vertexBuffer.buffer, offsets);
	// vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, ui->pipelineLayout, 0, 1, &ui->objects[0]->texture->descriptorSet, 0, nullptr);
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
