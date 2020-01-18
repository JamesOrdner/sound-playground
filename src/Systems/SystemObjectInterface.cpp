#include "SystemObjectInterface.h"

SystemObjectInterface::SystemObjectInterface(const SystemSceneInterface* scene, const UObject* uobject) :
	uobject(uobject),
	scene(scene)
{
}

