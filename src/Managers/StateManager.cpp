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
	eventQueue.push(Event{ EventKey(subject, event) , data });
}

void StateManager::eventImmediate(const SubjectInterface* subject, EventType event, const EventData& data, bool bEventFromParent)
{
	EventKey key(subject, event);
	for (const auto& observer : observers) {
		if (observer.key == key) {
			observer.callback(data, bEventFromParent);
		}
	}
}

StateManager::ObserverID tempCounter = 0; // TODO: ID generator
StateManager::ObserverID StateManager::registerObserver(
	const SubjectInterface* subject,
	EventType event,
	ObserverInterface::ObserverCallback callback)
{
	auto& observerData = observers.emplace_back();
	observerData.key = EventKey(subject, event);
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

	while (!eventQueue.empty()) {
		auto& event = eventQueue.front();
		for (const auto& observer : observers) {
			if (observer.key == event.key) {
				observer.callback(event.data, false);
			}
		}
		eventQueue.pop();
	}
}
