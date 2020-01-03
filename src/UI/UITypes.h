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
};

struct UIData
{
	// The callback to be called when the value is changed from the UI
	std::function<void(UIValue)> callback;

	// UI text label for this view
	std::string label;

	UIType type;
	UIValue value;
};
