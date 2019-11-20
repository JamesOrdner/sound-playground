#include "EModel.h"
#include "Engine.h"

EModel::EModel(const std::string& filepath) : modelMatrix(mat::mat4::Identity()), scale(1)
{
	mesh = Engine::instance().makeMesh(filepath);
}

std::string EModel::meshFilepath()
{
	return mesh->meshFilepath();
}

void EModel::setLocation(const mat::vec3& location)
{
	this->location = location;
	modelMatrix = mat::transform(location, scale);
}

const mat::vec3& EModel::getLocation()
{
	return location;
}

void EModel::setScale(float scale)
{
	this->scale = mat::vec3(scale);
	modelMatrix = mat::transform(location, this->scale);
}

void EModel::setScale(const mat::vec3& scale)
{
	this->scale = scale;
	modelMatrix = mat::transform(location, scale);
}

const mat::vec3& EModel::getScale()
{
	return scale;
}

void EModel::draw(unsigned int modelMatrixID)
{
	mesh->draw(modelMatrixID, modelMatrix);
}
