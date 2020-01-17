#pragma once

class SystemSceneInterface
{
public:

	SystemSceneInterface(const class UScene* uscene);

	virtual ~SystemSceneInterface() {};

	const class UScene* const uscene;

	// Create a system object and associate it with this system scene
	template<typename T>
	T* createSystemObject(const class UObject* uobject) {
		return static_cast<T*>(addSystemObject(new T(uobject)));
	}

private:

	virtual class SystemObjectInterface* addSystemObject(class SystemObjectInterface* object) = 0;
};
