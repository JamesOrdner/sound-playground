#pragma once

class SystemSceneInterface
{
public:

	virtual ~SystemSceneInterface() {};

	// Create a system object and associate it with this system scene
	template<class T>
	T* createSystemObject(const class UObject* uobject) {
		return static_cast<T*>(addSystemObject(new T(uobject)));
	};

private:

	// Take ownership of newly-created SystemObjectInterface object
	virtual class SystemObjectInterface* addSystemObject(class SystemObjectInterface* object) = 0;
};
