#pragma once

#include "../Util/Observer.h"
#include <list>
#include <memory>
#include <vector>

class UScene : public ObserverInterface, public SubjectInterface
{
public:

	UScene(const class Engine* engine);

	~UScene();

	// Create a UObject and associate it with this scene
	class UObject* createUniversalObject();

private:

	const class Engine* const engine;

	std::list<std::unique_ptr<class UObject>> uobjects;
};
