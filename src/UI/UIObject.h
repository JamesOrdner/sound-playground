#pragma once

#include <string>
#include <vector>
#include <functional>

struct UIObject
{
	std::string name;

	// Subobjects inherit the transform of the parent object
	std::vector<UIObject> subobjects;

	// Lower left corner [-1, 1]
	float x0, y0;

	// Upper right corner [-1, 1]
	float x1, y1;

	std::function<void()> callback;

	bool bAcceptsInput;
};
