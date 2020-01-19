#pragma once

#include "../Util/LFQueue.h"
#include "../Util/Observer.h"
#include <utility>
#include <map>
#include <list>

class StateManager
{
public:

	// Called by subjects after modifying shared data, executed asynchronously
	void event(const SubjectInterface* subject, EventType event, const EventData& data = EventData());

	// Called by subjects after modifying shared data, executed immediately
	void eventImmediate(const SubjectInterface* subject, EventType event, const EventData& data);

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

	struct ObserverData
	{
		ObserverInterface::ObserverCallback callback;
		const SubjectInterface* subject;
		EventType event;
		ObserverID id;
	};

	std::list<ObserverData> observers;

	// Queue stores pending observers which have requested unregistration
	LFQueue<ObserverID> removeQueue;

	typedef std::pair<const SubjectInterface*, EventType> EventKey;
	std::map<EventKey, EventData> eventQueue;

public:

	static StateManager& instance();
	
	// Deleted functions prevent singleton duplication
	StateManager(StateManager const&) = delete;
	void operator=(StateManager const&) = delete;
};
