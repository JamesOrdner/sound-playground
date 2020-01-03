#pragma once

#include <memory>

// Forward declarations
struct UIObject;
struct UICoord;
union SDL_Event;

class UIManager
{
public:

	UIManager();

	~UIManager();

	// Handles input events. Returns true if the UI consumed the input.
	bool handeInput(const SDL_Event& event, int screenWidth, int screenHeight);

	// This is the root of all drawn UIObjects. UIObjects are drawn in the order that they appear in the
	// UIObjects::subobjects array, and all of a UIObject's subobjects are drawn before the next UIObject in
	// the array. UIObjects draw on top of previously-drawn objects, so later-drawn objects will be on top.
	std::unique_ptr<UIObject> root;

private:

	void setupMenuBar();

	// Returns the object at screen coordinates (x, y) [-1, 1]
	UIObject* objectAt(const UICoord& location);

	UIObject* objectAtRecursive(
		UIObject& object,
		const UICoord& location,
		const UICoord& p_trans,
		const UICoord& p_scale);
};
