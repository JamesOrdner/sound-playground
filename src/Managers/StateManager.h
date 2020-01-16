#pragma once

#include "../Util/LFQueue.h"
#include "../Util/Observer.h"
#include <list>

class StateManager
{
public:

	// Called by subjects after modifying shared data
	void event(const SubjectInterface* subject, EventType event, const EventData& data = EventData());

	typedef unsigned int ObserverID;

	// Register an observer that will be notified after a call to notifyObservers()
	[[nodiscard]] ObserverID registerObserver(
		const SubjectInterface* subject,
		EventType event,
		ObserverInterface::ObserverCallback callback);

	// Register an observer that will be notified after a call to notifyAudioObservers()
	[[nodiscard]] ObserverID registerAudioObserver(
		const SubjectInterface* subject,
		EventType event,
		ObserverInterface::ObserverCallback callback);

	// Unregister an observer with the given ID
	void unregisterObserver(ObserverID id);
	void unregisterAudioObserver(ObserverID id);

	// Iterates pending events and dispatches to registered observers
	void notifyObservers();

	// Iterates pending events and dispatches to registered observers in the audio thread
	void notifyAudioObservers();

private:

	StateManager();

	struct ObserverData
	{
		ObserverInterface::ObserverCallback callback;
		const SubjectInterface* subject;
		EventType event;
		ObserverID id;
	};

	std::list<ObserverData> observers;

	// Observers needing notifications on the audio thread are stored here
	std::list<ObserverData> audioObservers;

	struct NotifyQueueItem
	{
		EventData data;
		const SubjectInterface* subject;
		EventType event;
	};

	// Stores all pending events
	LFQueue<NotifyQueueItem> eventQueue;

	// As the audio thread runs on its own callback-driven thread, it requires a parallel
	// queue that can be polled separately from the main event queue.
	LFQueue<NotifyQueueItem> audioEventQueue;

	// Queue stores pending observers which have requested unregistration
	LFQueue<ObserverID> removeQueue;
	LFQueue<ObserverID> audioRemoveQueue;

public:

	static StateManager& instance();
	
	// Deleted functions prevent singleton duplication
	StateManager(StateManager const&) = delete;
	void operator=(StateManager const&) = delete;
};
