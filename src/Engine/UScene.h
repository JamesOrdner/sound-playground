#pragma once

#include <list>
#include <memory>
#include <vector>

class UScene
{
public:

	~UScene();

	// Create a UObject and associate it with this scene
	class UObject* createUniversalObject();

private:

	std::list<std::unique_ptr<class UObject>> objects;

	std::vector<class SystemSceneInterface*> systemScenes;
};
