#pragma once

#include "../Util/Observer.h"
#include <vector>

class UObject : public SubjectInterface
{
public:

	UObject();
	
	std::vector<UObject*> subobjects;

private:

	// SubjectInterface
	void forwardEventImmediate(EventType event, const EventData& data) const override;
};
