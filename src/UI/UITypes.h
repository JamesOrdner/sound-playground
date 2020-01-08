#pragma once

#include <SDL_keyboard.h>
#include <string>
#include <functional>

enum class UIAnchor
{
	Center,
	Top,
	Bottom,
	Left,
	Right,
	TopLeft,
	TopRight,
	BottomLeft,
	BottomRight
};

enum class UIType
{
	Button,
	Slider
};

union UIValue
{
	SDL_Keycode keycode;
	float floatVal;
	bool boolVal;
};

// UIData is a data container used to expose UObject properties to the UI. It contains only
// the data relevant to UObjects. Drawing and event handling are all managed by UIManager.
struct UIData
{
	// The callback to be called when the value is changed from the UI
	std::function<void(UIValue)> callback;

	// UI text label for this view
	std::string label;

	UIType type;
	UIValue value;
};
