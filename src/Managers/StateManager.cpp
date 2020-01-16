#include "StateManager.h"

StateManager& StateManager::instance()
{
	static StateManager instance;
	return instance;
}

StateManager::StateManager()
{
}

void StateManager::event(const void* subject, EventType event, const EventData& data)
{
	NotifyQueueItem queueItem;
	queueItem.subject = subject;
	queueItem.event = event;
	queueItem.data = data;
	eventQueue.push(queueItem);
	audioEventQueue.push(queueItem);
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

StateManager::ObserverID StateManager::registerAudioObserver(const void* subject, EventType event, ObserverCallback callback)
{
	auto& observerData = audioObservers.emplace_back();
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

void StateManager::unregisterAudioObserver(ObserverID id)
{
	audioRemoveQueue.push(id);
}

void StateManager::notifyObservers()
{
	// Sticking this here for now, probably should go somewhere else
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

void StateManager::notifyAudioObservers()
{
	// Sticking this here for now, probably should go somewhere else
	ObserverID id;
	while (audioRemoveQueue.pop(id)) {
		audioObservers.remove_if([id](const ObserverData& data) { return data.id == id; });
	}

	NotifyQueueItem event;
	while (audioEventQueue.pop(event)) {
		for (const auto& observer : audioObservers) {
			if (event.subject == observer.subject && event.event == observer.event) {
				observer.callback(event.data);
			}
		}
	}
}
