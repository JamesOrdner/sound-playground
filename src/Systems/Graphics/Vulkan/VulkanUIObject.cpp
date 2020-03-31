#include "VulkanUIObject.h"
#include "VulkanInstance.h"

VulkanUIObject::VulkanUIObject(VulkanInstance* instance) :
	position{},
	bounds{},
	uv_position{},
	uv_bounds{},
	drawOrder(0),
	instance(instance),
	texture(nullptr)
{
}

void VulkanUIObject::setTexture(const std::string& filepath)
{
	texture = instance->sharedTexture(filepath);
}
