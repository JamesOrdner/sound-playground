#pragma once

#include "../Engine/SystemObjectInterface.h"

class PhysicsObject : public SystemObjectInterface
{
public:

	PhysicsObject(const class UObject* uobject);

	virtual ~PhysicsObject();
};
