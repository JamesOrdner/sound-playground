#pragma once

#include "../../Util/Matrix.h"

class GraphicsSystemInterface
{
public:

	virtual void screenDimensions(int& x, int& y) const = 0;

	virtual mat::mat4 screenToWorldTransform(const class UScene* uscene) const = 0;
};
