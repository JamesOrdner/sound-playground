#pragma once

#include <memory>

// Forward declarations
class UScene;
class SystemSceneInterface;

class SystemInterface
{
public:

	virtual ~SystemInterface() {};

	// Initialize the system
	virtual bool init() = 0;

	// Deinitialize the system
	virtual void deinit() = 0;

	// Execute a single run of the system, normally a single frame
	virtual void execute(float deltaTime) = 0;

	// Create a system scene and associate it with this system
	template<class T>
	T* createSystemScene(const UScene* uscene) {
		return static_cast<T*>(addSystemScene(new T, uscene));
	};

private:

	// Take ownership of newly-created SystemSceneInterface object
	virtual SystemSceneInterface* addSystemScene(SystemSceneInterface* scene, const UScene* uscene) = 0;
};
