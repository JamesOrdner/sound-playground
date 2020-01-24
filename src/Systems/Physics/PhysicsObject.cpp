#include "PhysicsObject.h"
#include "../../Engine/UObject.h"
#include "PhysicsMesh.h"

PhysicsObject::PhysicsObject(const SystemSceneInterface* scene, const UObject* uobject) :
	SystemObjectInterface(scene, uobject),
	scale(1),
	parentScale(1),
	bDirtyTransform(true),
	mesh(nullptr)
{
	registerCallback(
		uobject,
		EventType::PositionUpdated,
		[this](const EventData& data, bool bEventFromParent) {
			(bEventFromParent ? parentPosition : position) = std::get<mat::vec3>(data);
			this->uobject->childEventImmediate(EventType::PositionUpdated, position + parentPosition);
			bDirtyTransform = true;
		}
	);

	registerCallback(
		uobject,
		EventType::RotationUpdated,
		[this](const EventData& data, bool bEventFromParent) {
			(bEventFromParent ? parentRotation : rotation) = std::get<mat::vec3>(data);
			this->uobject->childEventImmediate(EventType::RotationUpdated, rotation + parentRotation);
			bDirtyTransform = true;
		}
	);

	registerCallback(
		uobject,
		EventType::ScaleUpdated,
		[this](const EventData& data, bool bEventFromParent) {
			(bEventFromParent ? parentScale : scale) = std::get<mat::vec3>(data);
			this->uobject->childEventImmediate(EventType::ScaleUpdated, scale * parentScale);
			bDirtyTransform = true;
		}
	);
}

PhysicsObject::~PhysicsObject()
{
}

void PhysicsObject::setPhysicsMesh(std::string filepath)
{
	mesh = PhysicsMesh::sharedMesh(filepath);
}

const mat::mat4& PhysicsObject::transformMatrix()
{
	if (bDirtyTransform) {
		transform = mat::transform(position + parentPosition, rotation + parentRotation, scale * parentScale);
		bDirtyTransform = false;
	}
	return transform;
}

void PhysicsObject::updateVelocity(float deltaTime)
{
	auto currentPosition = position + parentPosition;
	auto velocity = (currentPosition - previousPosition) / deltaTime;
	previousPosition = currentPosition;
	uobject->event(EventType::VelocityUpdated, velocity);
}

float PhysicsObject::raycast(const mat::vec3& origin, const mat::vec3& direction, mat::vec3& hit)
{
	using namespace mat;
	const std::vector<vec3>& buffer = mesh->buffer();

	// model space origin/direction
	mat4 iTransform = inverse(transformMatrix());
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
				hit = originModel + dirModel * t;
			}
		}
	}
	hit = vec3(transformMatrix() * vec4(hit, 1.f));
	return shortest > 0 ? dist(origin, hit) : -1.f;
}
