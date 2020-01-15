#pragma once

#include "../Engine/SystemSceneInterface.h"
#include <list>
#include <memory>

class InputScene : public SystemSceneInterface
{
public:

	~InputScene();

private:

	std::list<std::unique_ptr<class InputObject>> inputObjects;

	SystemObjectInterface* addSystemObject(SystemObjectInterface* object, const UObject* uobject) override;
};
