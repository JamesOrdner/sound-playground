#pragma once

#include "GraphicsObject.h"
#include "../Util/Matrix.h"

class CameraGraphicsObject : public GraphicsObject
{
public:

	CameraGraphicsObject(const class UObject* uobject);

	const mat::vec3& cameraPosition() const;

	mat::vec3 cameraForward() const;

private:

	mat::vec3 position;

	mat::vec3 rotation;
};
