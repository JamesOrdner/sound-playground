#include "EModel.h"
#include "../Engine/Engine.h"
#include "../Graphics/GMesh.h"
#include "../Engine/EInputComponent.h"

EModel::EModel() :
	mesh(nullptr),
	bDirtyTransform(false),
	bDirtySelection(false)
{
}

EModel::~EModel() = default;

void EModel::setMesh(std::string filepath)
{
	mesh = GMesh::getSharedMesh(filepath);
	mesh->registerModel(this);
}

float EModel::raycast(const mat::vec3& origin, const mat::vec3& direction, mat::vec3& hitLoc)
{
	using namespace mat;
	const std::vector<vec3>& buffer = mesh->getRayMesh();

	// model space origin/direction
	mat4 transform = transformMatrix();
	mat4 iTransform = inverse(transform);
	vec3 originModel = vec3(iTransform * vec4(origin, 1.f));
	vec3 dirModel = vec3(iTransform * vec4(direction, 0.f));

	// MT Raytrace
	float shortest = -1.f;
	for (size_t i = 0; i < buffer.size(); i += 3) {
		const vec3& v0 = buffer[i];
		const vec3& v1 = buffer[i + 1];
		const vec3& v2 = buffer[i + 2];
		vec3 edge1 = v1 - v0;
		vec3 edge2 = v2 - v0;
		vec3 h = cross(dirModel, edge2);
		float a = dot(edge1, h);
		if (a > -FLT_EPSILON && a < FLT_EPSILON) continue; // Ray is parallel to triangle
		float f = 1.f / a;
		vec3 s = originModel - v0;
		float u = f * dot(s, h);
		if (u < 0.f || u > 1.f) continue;
		vec3 q = cross(s, edge1);
		float v = f * dot(dirModel, q);
		if (v < 0.f || u + v > 1.f) continue;
		float t = f * dot(edge2, q);
		if (FLT_EPSILON < t && t < FLT_MAX) { // ray intersection
			if (shortest < 0 || t < shortest) {
				shortest = t;
				hitLoc = originModel + dirModel * t;
			}
		}
	}
	hitLoc = vec3(transform * vec4(hitLoc, 1.f));
	return shortest > 0 ? dist(origin, hitLoc) : -1.f;
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
		mat::vec3 dir = physicsPosition - m_position;
		setVelocity(dir / deltaTime);
		physicsPosition = m_position;
	}
	else {
		setVelocity(mat::vec3());
	}
}

mat::mat4 EModel::transformMatrix() const
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

void EModel::setSelected(bool selected)
{
	bDirtySelection = bDirtySelection || bSelected != selected;
	EObject::setSelected(selected);
}

bool EModel::needsSelectionUpdate()
{
	return bDirtySelection;
}

void EModel::selectionUpdated()
{
	bDirtySelection = true;
}
