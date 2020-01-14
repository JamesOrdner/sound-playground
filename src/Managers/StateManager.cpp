#include "StateManager.h"

StateManager& StateManager::instance()
{
	static StateManager instance;
	return instance;
}

StateManager::StateManager()
{
}

StateManager::ObserverID tempCounter = 0; // TODO: ID generator
StateManager::ObserverID StateManager::registerObserver(const void* subject, EventType event, ObserverCallback callback)
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

void StateManager::event(void* subject, EventType event, const EventData& data)
{
	NotifyQueueItem queueItem;
	queueItem.subject = subject;
	queueItem.event = event;
	queueItem.data = data;
	eventQueue.push(queueItem);
}

void StateManager::notifyObservers()
{
	// Sticking this here for now, should go somewhere else
	ObserverID id;
	while (removeQueue.pop(id)) {
		observers.remove_if([id](const ObserverData& data) { return data.id == id; });
	}

	NotifyQueueItem event;
	while (eventQueue.pop(event)) {
		for (const auto& observer : observers) {
			if (event.subject == observer.subject && event.event == observer.event) {
				observer.callback(event.data);
			}
		}
	}
}
