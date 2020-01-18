#pragma once

#include <vector>

class SystemObjectInterface
{
public:

	SystemObjectInterface(const class SystemSceneInterface* scene, const class UObject* uobject);

	virtual ~SystemObjectInterface() {};

	const class UObject* const uobject;

	// Pointer to the system scene that this object belongs to
	const class SystemSceneInterface* const scene;
};
