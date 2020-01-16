#pragma once

#include <vector>

class SystemObjectInterface
{
public:

	SystemObjectInterface(const class UObject* uobject);

	virtual ~SystemObjectInterface() {};

protected:

	const class UObject* const uobject;
};
