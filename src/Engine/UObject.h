#pragma once

#include <vector>

class UObject
{
public:

	typedef UObject* (*UObjectFactory)();

	template<typename T>
	static UObject* createObject()
	{
		return new T;
	}

private:

	// std::vector<class SystemObjectInterface*> systemObjects;
};
