#pragma once

#include <vector>

class SystemObjectInterface
{
public:

	SystemObjectInterface(const class UObject* uobject);

	virtual ~SystemObjectInterface() {};

	const class UObject* const uobject;
};
