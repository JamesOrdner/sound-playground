#pragma once

#include <SDL_events.h>
#include <utility>
#include <functional>
#include <map>

// Permissable SDL_EventTypes { SDL_KEYDOWN, SDL_KEYUP }
typedef std::pair<SDL_Keycode, SDL_EventType> EInputKeyEvent;

// Permissable SDL_EventTypes { SDL_MOUSEBUTTONDOWN, SDL_MOUSEBUTTONUP }
typedef std::pair<uint8_t, SDL_EventType> EInputMouseButtonEvent;

class EInputComponent
{
public:

	EInputComponent();

	void registerKeyCallback(EInputKeyEvent inputEvent, std::function<void()> callback);
	void deleteKeyCallback(EInputKeyEvent inputEvent);

	void registerMouseButtonCallback(EInputMouseButtonEvent inputEvent, std::function<void()> callback);
	void deleteMouseButtonCallback(EInputMouseButtonEvent inputEvent);

	void registerScrollUpCallback(std::function<void()> callback);
	void deleteScrollUpCallback();

	void registerScrollDownCallback(std::function<void()> callback);
	void deleteScrollDownCallback();

	// Register a function that will be called when a mouse moved event it received.
	// Callback takes the absolute pixel coordinates of the cursor, x and y.
	void registerCursorCallback(std::function<void(int, int)> callback);
	void deleteCursorCallback();

	void processInput(const SDL_Event& event);

private:

	std::map<EInputKeyEvent, std::function<void()>> keyCallbacks;
	std::map<EInputMouseButtonEvent, std::function<void()>> mouseButtonCallbacks;
	std::function<void()> scrollUpCallback;
	std::function<void()> scrollDownCallback;
	std::function<void(int, int)> cursorCallback;
};
