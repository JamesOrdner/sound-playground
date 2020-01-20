#pragma once

#include "../Util/Matrix.h"
#include "../Managers/AssetTypes.h"
#include "../UI/UITypes.h"
#include <memory>
#include <variant>
#include <functional>

enum class EventType
{
	CreateObjectRequest, // EventData type: CreateObjectRequestData
	PositionUpdated,     // EventData type: mat::vec3
	VelocityUpdated,     // EventData type: mat::vec3
	RotationUpdated,     // EventData type: mat::vec3
	ScaleUpdated,        // EventData type: mat::vec3
	SelectionUpdated,    // EventData type: bool

	// UI
	CreateUIObjectRequest,    // EventData type: CreateObjectRequestData
	UIPositionUpdated,        // EventData type: mat::vec2
	UIBoundsUpdated,          // EventData type: mat::vec2
	UIDrawOrderUpdated,       // EventData type: uint32_t
	UITextureAssetUpdated,    // EventData type: AssetID
	UITexturePositionUpdated, // EventData type: mat::vec2
	UITextureBoundsUpdated    // EventData type: mat::vec2
};

// If not nullptr, this function will be called immediately after the UObject is created
// First parameter is the created UObject. Second parameter is the forwarded userData.
typedef void(*CreateObjectCallback)(class UObject*, void*);

struct CreateObjectRequestData
{
	AssetID assetID;
	void* userData;
	CreateObjectCallback callback;
};

// This variant includes all possible event callback data types. As event data for all event
// types will have enough memory allocated to store the largest type, these should be small
typedef std::variant<bool, uint32_t, mat::vec2, mat::vec3, AssetID, CreateObjectRequestData> EventData;

class ObserverInterface
{
public:

	ObserverInterface();

	virtual ~ObserverInterface();

	typedef std::function<void(const EventData& data, bool bEventFromParent)> ObserverCallback;
	void registerCallback(const class SubjectInterface* subject, EventType event, ObserverCallback&& callback);

private:

	std::unique_ptr<struct ObserverData> data;
};

class SubjectInterface
{
public:

	SubjectInterface();

	virtual ~SubjectInterface();

	// Produce an event to execute asynchronously
	void event(EventType event, const EventData& data = EventData()) const;

	// Execute an event synchronously
	void eventImmediate(EventType event, const EventData& data, bool bEventFromParent = false) const;

private:

	std::unique_ptr<struct SubjectData> data;
};
