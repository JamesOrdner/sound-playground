#pragma once

#include "../Graphics/Matrix.h"
#include "../Util/LFQueue.h"
#include <list>
#include <functional>
#include <variant>

// Forward declarations
class AudioComponent; // TODO: Generalize to a base component type

class StateManager
{
public:

	// This variant includes all possible callback parameter types
	typedef std::variant<
		float,
		mat::vec3,
		AudioComponent*
	> EventData;

	typedef std::function<void(const EventData&)> ObserverCallback;

	enum class EventType
	{
		ComponentCreated, // AudioComponent*
		ComponentDeleted, // AudioComponent*
		PositionUpdated, // mat::vec3
		VelocityUpdated, // mat::vec3
		RotationUpdated, // mat::vec3
		ScaleUpdated, // mat::vec3
	};

	typedef unsigned int ObserverID;

	ObserverID registerObserver(const void* subject, EventType event, ObserverCallback callback);
	void unregisterObserver(ObserverID id);

	// Called by subjects after modifying shared data
	void event(void* subject, EventType event, const EventData& data = EventData());

	// Iterates pending events and dispatches to registered observers
	void notifyObservers();

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

	struct NotifyQueueItem
	{
		EventData data;
		void* subject;
		EventType event;
	};

	LFQueue<NotifyQueueItem> eventQueue;

	// Queue stores pending observers which have requested unregistration
	LFQueue<ObserverID> removeQueue;

public:

	static StateManager& instance();
	
	// Deleted functions prevent singleton duplication
	StateManager(StateManager const&) = delete;
	void operator=(StateManager const&) = delete;
};
