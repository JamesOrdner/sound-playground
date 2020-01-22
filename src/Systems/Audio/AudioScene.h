#pragma once

#include "../SystemSceneInterface.h"
#include <list>
#include <memory>

class AudioScene : public SystemSceneInterface
{
public:

	AudioScene(const class SystemInterface* system, const class UScene* uscene);

	~AudioScene();

	void deleteSystemObject(const class UObject* uobject) override;

private:

	std::list<std::unique_ptr<class AudioObject>> audioObjects;

	SystemObjectInterface* addSystemObject(SystemObjectInterface* object) override;
};
