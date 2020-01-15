#pragma once

#include "../Managers/StateManager.h"
#include <vector>

class EComponent
{
public:

	EComponent();

	virtual ~EComponent();

protected:

	// Each time this component registers as an observer, it should add the returned
	// ObserverID to this vector to allow automatic unregistration on deallocation
	std::vector<StateManager::ObserverID> observerIDs;
};

