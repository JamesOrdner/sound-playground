#pragma once

#include "../Util/LFQueue.h"
#include "../Util/Observer.h"
#include <utility>
#include <queue>
#include <list>

class StateManager
{
public:

	// Called by subjects after modifying shared data, executed asynchronously
	void event(const SubjectInterface* subject, EventType event, const EventData& data = EventData());

	// Called by subjects after modifying shared data, executed immediately
	void eventImmediate(const SubjectInterface* subject, EventType event, const EventData& data, bool bEventFromParent = false);

	typedef unsigned int ObserverID;

	// Register an observer that will be notified after a call to notifyObservers()
	[[nodiscard]] ObserverID registerObserver(
		const SubjectInterface* subject,
		EventType event,
		ObserverInterface::ObserverCallback callback);

	// Unregister an observer with the given ID
	void unregisterObserver(ObserverID id);

	// Iterates pending events and dispatches to registered observers
	void notifyObservers();

private:

	StateManager();

	typedef std::pair<const SubjectInterface*, EventType> EventKey;

	struct ObserverData
	{
		ObserverInterface::ObserverCallback callback;
		EventKey key;
		ObserverID id;

		ObserverData() : id() {}
	};
	std::list<ObserverData> observers;

	struct Event
	{
		EventKey key;
		EventData data;
	};
	std::queue<Event> eventQueue;

	// Stores pending observers which have requested unregistration
	LFQueue<ObserverID> removeQueue;

public:

	static StateManager& instance();
	
	// Deleted functions prevent singleton duplication
	StateManager(StateManager const&) = delete;
	void operator=(StateManager const&) = delete;
};
