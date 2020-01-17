#pragma once

#include "../SystemObjectInterface.h"

class GraphicsObject : public SystemObjectInterface
{
public:

	GraphicsObject(const class UObject* uobject);

	virtual ~GraphicsObject();
};
