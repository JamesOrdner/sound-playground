#include "EInputComponent.h"

void EInputComponent::registerCallback(SDL_Keycode keycode, std::function<void()> callback)
{
	callbacks[keycode] = callback;
}

void EInputComponent::deleteCallback(SDL_Keycode keycode)
{
	callbacks.erase(keycode);
}

void EInputComponent::processInput(SDL_Keycode keycode)
{
	auto callback = callbacks.find(keycode);
	if (callback != callbacks.end()) callback->second();
}
