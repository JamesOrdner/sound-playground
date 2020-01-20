#pragma once

#include "UITypes.h"
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

	// Subobjects inherit the transform of the parent object
	std::vector<UIObject> subobjects;

	// Shared data, absolute (parentless), replicated to graphics system
	UIObjectData replicatedData;

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

	// Rate of object animation. Set to 0 if object does not animate (default).
	float animationRate;

	// Arbitrary data assignable by the InputSystem
	void* userData;

	// Sets the animation target position of the object. If the object does not animate,
	// it will be set immediately to this position.
	void setAnimationTarget(const mat::vec2& position);

	// Returns the relative anchor position in normalized device coordinates [-1, 1]
	mat::vec2 anchorPosition() const;

	// Tick this object and all subobjects
	void tick(float deltaTime);

private:

	mat::vec2 animationTarget;
};
