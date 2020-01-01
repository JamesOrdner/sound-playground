#pragma once

#include <SDL_keyboard.h>
#include <string>
#include <functional>

enum class UIViewType
{
	button,
	slider
};

union UIViewValue
{
	SDL_Keycode keycode;
	float floatVal;
};

struct UIViewData
{
	// The callback to be called when the value is changed from the UI
	std::function<void(float)> callback;

	// UI text descriptor for this view
	std::string label;

	UIViewType type;
	UIViewValue value;
};
