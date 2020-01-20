#pragma once

#include "../Util/Matrix.h"
#include "../Managers/AssetTypes.h"
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
	Slider,
	None
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

	UIData() :
		type(UIType::None),
		value{}
	{
	}
};

// This struct contains all the data required to render the object, i.e. all data replicated between Systems.
// All data is parentless and absolute, relative only to the virtual resolution of the UI
struct UIObjectData
{
	// Position offset of the anchor point of the object, in pixels
	mat::vec2 position;

	// Absoule size of the object in pixels
	mat::vec2 bounds;

	// Draw order of this UIObject. Larger order values are drawn on top of lower order values.
	uint32_t drawOrder;

	// Texture used to render this UI object
	AssetID textureAsset;

	// UV position of the top left corner of this object in the UI texture, in pixels.
	mat::vec2 texturePosition;

	// UV bounds of this object in the UI texture, in pixels.
	mat::vec2 textureBounds;

	UIObjectData() : drawOrder(), textureAsset() {}
};
