#pragma once

#include "UITypes.h"
#include "../Graphics/Matrix.h"
#include <string>
#include <vector>
#include <functional>
#include <optional>

enum class UIObjectState
{
	Neutral,
	Hovered,
	Selected
};

struct UIObject
{
	UIObject();

	std::string name;

	UIValue value;

	// Subobjects inherit the transform of the parent object
	std::vector<UIObject> subobjects;

	// Anchor point of the object. This affects both what is considered the "center" point
	// of the object, as well as where the object is positioned relative to the parent.
	UIAnchor anchor;

	// Position offset of the anchor point of the object, in pixels
	mat::vec2 position;

	// Absoule size of the object in pixels
	mat::vec2 bounds;

	// UV coordinates of this object to the UI texture, in pixels.
	// Implemented as custom function to allow state-dependent textures.
	// { x: top left, y: top left, width, height }
	std::function<mat::vec4()> textureCoords;

	// Function called when this object is interacted with
	std::function<void(struct UIManagerEvent& uiEvent)> callback;

	// Does this object accept and consume input? If bAcceptsInput == true and no callback
	// function exists, the object will simply consume input without any resulting action.
	bool bAcceptsInput;

	// Current state of the object
	UIObjectState state;

	// Returns the relative anchor position in normalized device coordinates [-1, 1]
	mat::vec2 anchorPosition() const;
};
