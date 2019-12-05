#include "EModel.h"
#include "../Graphics/GMesh.h"
#include "../Audio/AudioComponent.h"

EModel::EModel(const std::string& filepath) : bDirtyTransform(false)
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
	EObject::setPosition(location);
	bDirtyTransform = true;
}

void EModel::setRotation(const mat::vec3& rotation)
{
	EObject::setRotation(rotation);
	bDirtyTransform = true;
}

void EModel::setScale(float scale)
{
	EObject::setScale(scale);
	bDirtyTransform = true;
}

void EModel::setScale(const mat::vec3& scale)
{
	EObject::setScale(scale);
	bDirtyTransform = true;
}

void EModel::updatePhysics(float deltaTime)
{
	if (m_position != physicsPosition) {
		if (AudioComponent* ac = audioComponent()) {
			mat::vec3 dir = physicsPosition - m_position;
			ac->updateVelocity(dir / deltaTime);
		}
		physicsPosition = m_position;
	}
	else {
		if (AudioComponent* ac = audioComponent()) {
			ac->updateVelocity(mat::vec3());
		}
	}
}

mat::mat4 EModel::transformMatrix()
{
	return mat::transform(m_position, m_rotation, m_scale);
}

bool EModel::needsTransformUpdate()
{
	return bDirtyTransform;
}

void EModel::transformUpdated()
{
	bDirtyTransform = false;
}
