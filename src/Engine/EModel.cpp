#include "EModel.h"
#include "../Graphics/GMesh.h"

EModel::EModel(const std::string& filepath) : scale(1), bDirtyTransform(false)
{
	this->filepath = filepath;
}

std::string EModel::getFilepath()
{
	return filepath;
}

void EModel::registerWithMesh(std::shared_ptr<GMesh> mesh)
{
	this->mesh = mesh;
}

void EModel::unregister()
{
	mesh.reset();
}

std::shared_ptr<GMesh> EModel::getMesh()
{
	return mesh;
}

void EModel::setPosition(const mat::vec3& location)
{
	this->position = location;
	bDirtyTransform = true;
}

const mat::vec3& EModel::getPosition()
{
	return position;
}

void EModel::setScale(float scale)
{
	this->scale = mat::vec3(scale);
	bDirtyTransform = true;
}

void EModel::setScale(const mat::vec3& scale)
{
	this->scale = scale;
	bDirtyTransform = true;
}

const mat::vec3& EModel::getScale()
{
	return scale;
}

bool EModel::needsTransformUpdate()
{
	return bDirtyTransform;
}

void EModel::transformUpdated()
{
	bDirtyTransform = false;
}
