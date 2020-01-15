#pragma once

// Forward declarations
class UObject;
class SystemObjectInterface;

class SystemSceneInterface
{
public:

	virtual ~SystemSceneInterface() {};

	// Create a system object and associate it with this system scene
	template<class T>
	T* createSystemObject(const UObject* uobject) {
		return static_cast<T*>(addSystemObject(new T, uobject));
	};

private:

	// Take ownership of newly-created SystemObjectInterface object
	virtual SystemObjectInterface* addSystemObject(SystemObjectInterface* object, const UObject* uobject) = 0;
};
