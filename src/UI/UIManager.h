#pragma once

#include "../Util/Matrix.h"
#include <SDL_events.h>
#include <vector>
#include <memory>

// UIManagerEvent is the return value of handleInput(...), used as a
// means to communicate the result of an event to dependent processes
struct UIManagerEvent
{
	// Should this event consume the input?
	bool bConsumedInput;
	
	// Points to the object spawned by the event, or nullptr if no object spawned
	class InputObject* spawned;
};

// tempowary
struct UIObject;
struct UIData;

class UIManager
{
public:

	UIManager();

	~UIManager();

	// Virtual screen bounds. All UI elements are arranged to this virtual resolution.
	// Scaling to the actual screen resolution is handled automatically during rendering.
	static mat::vec2 screenBounds;

	// This is the root of all drawn UIObjects. UIObjects are drawn in the order that they appear in the
	// UIObjects::subobjects array, and all of a UIObject's subobjects are drawn before the next UIObject in
	// the array. UIObjects draw on top of previously-drawn objects, so later-drawn objects will be on top.
	std::unique_ptr<UIObject> root;

	// Points to the root object of the properties panel
	UIObject* propertiesRoot;
	
	// Data currently displayed in the properties panel
	std::vector<UIData>* propertiesData;

	// Handles input events. Returns true if the UI consumed the input.
	UIManagerEvent handeInput(const SDL_Event& event);

	// Set the data to be displayed in the properties panel, or nullptr to clear
	void setActiveData(std::vector<UIData>* data);

	// UIManager needs to be ticked to allow animations
	void tick(float deltaTime);

private:

	// Currently hovered object. Objects must accept input to become hovered.
	UIObject* hoveredObject;

	inline mat::vec2 virtualMousePosition(const SDL_Event& event) {
		return mat::vec2{ event.motion.x / 1280.f, 1.f - event.motion.y / 720.f } * screenBounds;
	}

	void setupMenuBar();
	void setupProperties();

	// Returns the object at location, provided as a virtual screen coordinate
	UIObject* objectAt(const mat::vec2& location);

	UIObject* objectAtRecursive(
		UIObject& object,
		const mat::vec2& location,
		const mat::vec2& parentCenterAbs,
		const mat::vec2& parentBoundsAbs);
};
