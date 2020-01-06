#pragma once

#include "UITypes.h"
#include "../Graphics/Matrix.h"
#include <string>
#include <vector>
#include <functional>

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

	// UV coordinates of this object to the UI texture, in pixels { x0, y0, x1, y1 } 
	mat::vec4 textureCoords;

	std::function<void()> callback;

	bool bAcceptsInput;

	// Returns the relative anchor position in normalized device coordinates [-1, 1]
	mat::vec2 anchorPosition() const;
};
