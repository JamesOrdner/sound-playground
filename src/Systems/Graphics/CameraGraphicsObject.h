#pragma once

#include "GraphicsObject.h"
#include "../../Util/Matrix.h"
#include "../../Util/Observer.h"

class CameraGraphicsObject : public GraphicsObject, public ObserverInterface
{
public:

	CameraGraphicsObject(const class SystemSceneInterface* scene, const class UObject* uobject);

	const mat::vec3& cameraPosition() const;

	mat::vec3 cameraForward() const;

private:

	mat::vec3 position;

	mat::vec3 rotation;
};
