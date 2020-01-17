#pragma once

#include "../SystemObjectInterface.h"

class PhysicsObject : public SystemObjectInterface
{
public:

	PhysicsObject(const class UObject* uobject);

	virtual ~PhysicsObject();
};
