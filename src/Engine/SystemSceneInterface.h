#pragma once

class SystemSceneInterface
{
public:

	SystemSceneInterface(const class SystemInterface* system, const class UScene* uscene);

	virtual ~SystemSceneInterface() {};

	// Pointer to the system that this scene belongs to
	const class SystemInterface* const system;

	// Pointer to the UScene that this system scene is associated with
	const class UScene* const uscene;

	// Create a system object and associate it with this system scene
	template<typename T>
	T* createSystemObject(const class UObject* uobject) {
		return static_cast<T*>(addSystemObject(new T(uobject)));
	}

private:

	// Take ownership of the newly-created SystemObject
	virtual class SystemObjectInterface* addSystemObject(class SystemObjectInterface* object) = 0;
};
