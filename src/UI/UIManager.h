#pragma once

#include "../Util/Matrix.h"
#include "../Managers/AssetTypes.h"
#include <SDL_events.h>
#include <vector>
#include <memory>
#include <queue>

// UIManagerEvent is the return value of handleInput(...), used as a
// means to communicate the result of an event to dependent processes
struct UIManagerEvent
{
	enum class Type
	{
		None,
		SpawnObject
	} type;

	// Should this event consume the input?
	bool bConsumedInput;
	
	struct UIObject* modified;

	// If Type == SpawnObject, this holds the requested AssetID
	AssetID spawnID;
};

// UIObjectModification stores the details of a single modification of a
// UIObject, allowing the owner of UIManager to handle modifications
struct UIObjectModification
{
	enum class Type
	{
		ObjectCreated,
		AnchorUpdated,
		PositionUpdated,
		BoundsUpdated,
		DrawOrderUpdated,
		TextureUpdated,
		TexturePositionUpdated,
		TextureBoundsUpdated
	} modification;

	struct UIObject* object;
};

class UIManager
{
public:

	UIManager(const class AssetManagerInterface* assetManager);

	~UIManager();

	// Virtual screen bounds. All UI elements are arranged to this virtual resolution.
	// Scaling to the actual screen resolution is handled automatically during rendering.
	static mat::vec2 screenBounds;

	// This is the root of all drawn UIObjects. UIObjects are drawn in the order that they appear in the
	// UIObjects::subobjects array, and all of a UIObject's subobjects are drawn before the next UIObject in
	// the array. UIObjects draw on top of previously-drawn objects, so later-drawn objects will be on top.
	std::unique_ptr<struct UIObject> root;

	// This queue stores all pending modifications to UIObjects, to be handled by the owner of UIManager
	std::queue<UIObjectModification> modificationQueue;

	// Handles input events. Returns true if the UI consumed the input.
	UIManagerEvent handeInput(const SDL_Event& event);

	// Set the data to be displayed in the properties panel, or nullptr to clear
	void setActiveData(std::vector<struct UIData>* data);

	// UIManager needs to be ticked to allow animations
	void tick(float deltaTime);

private:

	const class AssetManagerInterface* const assetManager;

	// Currently hovered object. Objects must accept input to become hovered.
	struct UIObject* hoveredObject;

	// Points to the root object of the properties panel
	struct UIObject* propertiesRoot;

	// Data currently displayed in the properties panel
	std::vector<struct UIData>* propertiesData;

	inline mat::vec2 virtualMousePosition(const SDL_Event& event) {
		return mat::vec2{ event.motion.x / 1280.f, 1.f - event.motion.y / 720.f } * screenBounds;
	}

	void setupMenuBar();
	void setupProperties();

	// Create a new UIObject. This should always be used instead of directly modifying a
	// UIObject's subobjects field, as it also communicates the creation of a new UIObject.
	struct UIObject& createUIObject(struct UIObject& parent);

	// Update all UIObjectData, inserting events into modificationQueue as needed
	void updateAllReplicatedData();
	void updateAllReplicatedData(struct UIObject& object, struct UIObject& parent);

	// Iterates entire UIObject hierarchy and assigns proper draw ordering
	void assignDrawOrders(struct UIObject& rootObject, uint32_t& currentOrderIndex);

	// Returns the object at location, provided as a virtual screen coordinate
	struct UIObject* objectAt(const mat::vec2& location);

	struct UIObject* objectAtRecursive(
		struct UIObject& object,
		const mat::vec2& location,
		const mat::vec2& parentCenterAbs,
		const mat::vec2& parentBoundsAbs);
};
