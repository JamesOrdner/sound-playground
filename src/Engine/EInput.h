#pragma once

#include <SDL_events.h>
#include <unordered_set>

// Forward declarations
struct UIManagerEvent;
class EObject;

class EInput
{
public:

	EInput();

	void handleInput(const SDL_Event& sdlEvent, const UIManagerEvent& uiEvent);

private:

	std::unordered_set<EObject*> selectedObjects;

	// True when selected objects should be placed at the cursor
	bool bPlacingSelected;
};
