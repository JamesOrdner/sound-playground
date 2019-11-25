#include "EObject.h"

EObject::EObject() : scale(1)
{

}

void EObject::setPosition(const mat::vec3& location)
{
	this->position = location;
}

const mat::vec3& EObject::getPosition()
{
	return position;
}

void EObject::setScale(float scale)
{
	this->scale = mat::vec3(scale);
}

void EObject::setScale(const mat::vec3& scale)
{
	this->scale = scale;
}

const mat::vec3& EObject::getScale()
{
	return scale;
}