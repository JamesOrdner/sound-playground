#include "StateManager.h"

StateManager& StateManager::instance()
{
	static StateManager instance;
	return instance;
}

StateManager::StateManager()
{
}

void StateManager::event(const SubjectInterface* subject, EventType event, const EventData& data)
{
	eventQueue[EventKey(subject, event)] = data;
}

StateManager::ObserverID tempCounter = 0; // TODO: ID generator
StateManager::ObserverID StateManager::registerObserver(
	const SubjectInterface* subject,
	EventType event,
	ObserverInterface::ObserverCallback callback)
{
	auto& observerData = observers.emplace_back();
	observerData.subject = subject;
	observerData.event = event;
	observerData.callback = callback;
	observerData.id = tempCounter++;
	return observerData.id;
}

void StateManager::unregisterObserver(ObserverID id)
{
	removeQueue.push(id);
}

void StateManager::notifyObservers()
{
	// Sticking this here for now, probably should go somewhere else
	ObserverID id;
	while (removeQueue.pop(id)) {
		observers.remove_if([id](const ObserverData& data) { return data.id == id; });
	}

	for (const auto& event : eventQueue) {
		for (const auto& observer : observers) {
			if (event.first.first == observer.subject && event.first.second == observer.event) {
				observer.callback(event.second);
			}
		}
	}
	eventQueue.clear();
}
