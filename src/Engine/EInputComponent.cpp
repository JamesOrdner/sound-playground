#include "EInputComponent.h"

EInputComponent::EInputComponent() :
	prevCursorX(0),
	prevCursorY(0)
{
}

void EInputComponent::registerKeyCallback(EInputKeyEvent inputEvent, std::function<void()> callback)
{
	keyCallbacks[inputEvent] = callback;
}

void EInputComponent::deleteKeyCallback(EInputKeyEvent inputEvent)
{
	keyCallbacks.erase(inputEvent);
}

void EInputComponent::registerMouseButtonCallback(EInputMouseButtonEvent inputEvent, std::function<void()> callback)
{
	mouseButtonCallbacks[inputEvent] = callback;
}

void EInputComponent::deleteMouseButtonCallback(EInputMouseButtonEvent inputEvent)
{
	mouseButtonCallbacks.erase(inputEvent);
}

void EInputComponent::registerScrollUpCallback(std::function<void()> callback)
{
	scrollUpCallback = callback;
}

void EInputComponent::deleteScrollUpCallback()
{
	scrollUpCallback = nullptr;
}

void EInputComponent::registerScrollDownCallback(std::function<void()> callback)
{
	scrollDownCallback = callback;
}

void EInputComponent::deleteScrollDownCallback()
{
	scrollDownCallback = nullptr;
}

void EInputComponent::registerCursorCallback(std::function<void(int, int)> callback)
{
	cursorCallback = callback;
}

void EInputComponent::deleteCursorCallback()
{
	cursorCallback = nullptr;
}

void EInputComponent::processInput(const SDL_Event& event)
{
	if (event.type == SDL_KEYDOWN || event.type == SDL_KEYUP) {
		EInputKeyEvent event_ = std::make_pair(event.key.keysym.sym, static_cast<SDL_EventType>(event.type));
		auto callback = keyCallbacks.find(event_);
		if (callback != keyCallbacks.end()) callback->second();
	}
	else if (event.type == SDL_MOUSEMOTION && cursorCallback) {
		int x, y;
		SDL_GetMouseState(&x, &y);
		cursorCallback(x - prevCursorX, y - prevCursorY);
		prevCursorX = x;
		prevCursorY = y;
	}
	else if (event.type == SDL_MOUSEBUTTONDOWN || event.type == SDL_MOUSEBUTTONUP) {
		EInputKeyEvent event_ = std::make_pair(event.button.button, static_cast<SDL_EventType>(event.type));
		auto callback = mouseButtonCallbacks.find(event_);
		if (callback != mouseButtonCallbacks.end()) callback->second();
	}
	else if (event.type == SDL_MOUSEWHEEL) {
		if (event.wheel.y > 0) {
			scrollUpCallback();
		}
		else if (event.wheel.y < 0) {
			scrollDownCallback();
		}
	}
}
