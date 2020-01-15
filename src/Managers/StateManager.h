#pragma once

#include "../Graphics/Matrix.h"
#include "../Util/LFQueue.h"
#include <list>
#include <functional>
#include <variant>

// Forward declarations
class EComponent;

class StateManager
{
public:

	// This variant includes all possible callback parameter types
	typedef std::variant<
		bool,
		mat::vec3,
		EComponent*
	> EventData;

	typedef std::function<void(const EventData&)> ObserverCallback;

	enum class EventType
	{
		ComponentCreated, // EComponent*
		ComponentDeleted, // EComponent*
		PositionUpdated, // mat::vec3
		VelocityUpdated, // mat::vec3
		RotationUpdated, // mat::vec3
		ScaleUpdated, // mat::vec3
		SelectionUpdated, // bool
	};

	// Called by subjects after modifying shared data
	void event(void* subject, EventType event, const EventData& data = EventData());

	typedef unsigned int ObserverID;

	// Register an observer that will be notified after a call to notifyObservers()
	ObserverID registerObserver(const void* subject, EventType event, ObserverCallback callback);

	// Register an observer that will be notified after a call to notifyAudioObservers()
	ObserverID registerAudioObserver(const void* subject, EventType event, ObserverCallback callback);

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
		ObserverCallback callback;
		const void* subject;
		EventType event;
		ObserverID id;
	};

	std::list<ObserverData> observers;

	// Observers needing notifications on the audio thread are stored here
	std::list<ObserverData> audioObservers;

	struct NotifyQueueItem
	{
		EventData data;
		void* subject;
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
