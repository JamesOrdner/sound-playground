#pragma once

#include "../Engine/SystemObjectInterface.h"

class GraphicsObject : public SystemObjectInterface
{
public:

	GraphicsObject(const class UObject* uobject);

	virtual ~GraphicsObject();
};
