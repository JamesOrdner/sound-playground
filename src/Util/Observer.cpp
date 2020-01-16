#include "Observer.h"
#include "../Managers/StateManager.h"
#include <vector>

struct ObserverData
{
	std::vector<StateManager::ObserverID> ids;
};

ObserverInterface::ObserverInterface()
{
	data = std::make_unique<ObserverData>();
}

ObserverInterface::~ObserverInterface()
{
	auto& stateManager = StateManager::instance();
	for (auto& id : data->ids) stateManager.unregisterObserver(id);
}

void ObserverInterface::registerCallback(const SubjectInterface* subject, EventType event, ObserverCallback&& callback)
{
	data->ids.push_back(StateManager::instance().registerObserver(subject, event, callback));
}

struct SubjectData
{
	// EventType data to filter events sent to StateManager
};

SubjectInterface::SubjectInterface()
{
	data = std::make_unique<SubjectData>();
}

SubjectInterface::~SubjectInterface()
{
}

void SubjectInterface::event(EventType event, const EventData& data) const
{
	StateManager::instance().event(this, event, data);
}
