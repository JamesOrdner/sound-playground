#pragma once

#include "../Util/Observer.h"
#include <vector>

class UObject : public SubjectInterface
{
public:

	typedef UObject* (*UObjectFactory)();

	template<typename T>
	static UObject* createObject()
	{
		return new T;
	}

	virtual ~UObject();
};
