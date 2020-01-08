#pragma once

#include <SDL_events.h>
#include <unordered_set>

// Forward declarations
class UIManager;
class EObject;

class EInput
{
public:

	EInput();

	// Pointer to the UIManager object
	UIManager* uiManager;

	void handleInput(const SDL_Event& sdlEvent);

private:

	std::unordered_set<EObject*> selectedObjects;

	// True when selected objects should be placed at the cursor
	bool bPlacingSelected;
};
