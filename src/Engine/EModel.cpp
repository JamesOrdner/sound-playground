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

float EModel::raycast(const mat::vec3& origin, const mat::vec3& direction, mat::vec3& hitLoc)
{
	using namespace mat;
	const std::vector<vec3>& rawBuffer = mesh->getRayMesh();

	// transformed buffer
	std::vector<vec3> buffer;
	buffer.reserve(rawBuffer.size());
	mat4 transform = transformMatrix();
	for (size_t i = 0; i < rawBuffer.size(); i++) {
		buffer.push_back(vec3(transform * vec4(rawBuffer[i])));
	}

	// MT Raytrace
	float shortest = -1.f;
	for (size_t i = 0; i < buffer.size(); i += 3) {
		const vec3& v0 = buffer[i];
		const vec3& v1 = buffer[i + 1];
		const vec3& v2 = buffer[i + 2];
		vec3 edge1 = v1 - v0;
		vec3 edge2 = v2 - v0;
		vec3 h = cross(direction, edge2);
		float a = dot(edge1, h);
		if (a > -FLT_EPSILON && a < FLT_EPSILON) continue; // Ray is parallel to triangle
		float f = 1.f / a;
		vec3 s = origin - v0;
		float u = f * dot(s, h);
		if (u < 0.f || u > 1.f) continue;
		vec3 q = cross(s, edge1);
		float v = f * dot(direction, q);
		if (v < 0.f || u + v > 1.f) continue;
		float t = f * dot(edge2, q);
		if (FLT_EPSILON < t && t < FLT_MAX) { // ray intersection
			if (shortest < 0 || t < shortest) {
				shortest = t;
				hitLoc = origin + direction * t;
			}
		}
	}
	return shortest;
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

mat::mat4 EModel::transformMatrix()
{
	return mat::mat4{
		{ scale.x,       0,       0, position.x * scale.x },
		{       0, scale.y,       0, position.y * scale.y },
		{       0,       0, scale.z, position.z * scale.z },
		{       0,       0,       0,                    1 }
	};
}

bool EModel::needsTransformUpdate()
{
	return bDirtyTransform;
}

void EModel::transformUpdated()
{
	bDirtyTransform = false;
}
