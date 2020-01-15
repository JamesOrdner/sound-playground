#pragma once

#include <list>
#include <memory>
#include <vector>

class UScene
{
public:

	~UScene();

	// Create a UObject and associate it with this system
	template<class T>
	T* createUniversalObject() {
		return static_cast<T*>(addUniversalObject(new T));
	};

private:

	std::list<std::unique_ptr<class UObject>> objects;

	std::vector<class SystemSceneInterface*> systemScenes;

	// Initialize and take ownership of newly-created UObject
	class UObject* addUniversalObject(class UObject* object);
};
