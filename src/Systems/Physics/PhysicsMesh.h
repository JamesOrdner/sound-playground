#pragma once

#include "../../Util/Matrix.h"
#include <map>
#include <string>
#include <memory>
#include <vector>

class PhysicsMesh
{
public:

	PhysicsMesh(const std::string& filepath);

	~PhysicsMesh();

	// Returns a pointer to a mesh object at the specified filepath.
	static PhysicsMesh* sharedMesh(const std::string& filepath);

	const std::vector<mat::vec3>& buffer() const;

private:

	// Stores all loaded physics meshes, indexed by path
	static std::map<std::string, std::unique_ptr<PhysicsMesh>> meshes;

	// The physics mesh buffer. Each three vertices forms a triangle.
	std::vector<mat::vec3> mesh;
};
