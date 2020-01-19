#pragma once

#include "../Util/Observer.h"
#include <vector>

class UObject : public SubjectInterface
{
public:

	UObject();
	
	std::vector<UObject*> subobjects;

	// Synchronously execute an event on all subobjects
	void childEventImmediate(EventType event, const EventData& data) const;
};
