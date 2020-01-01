#pragma once

#include <SDL_keyboard.h>
#include <functional>
#include <map>

class EInputComponent
{
public:

	void registerCallback(SDL_Keycode keycode, std::function<void()> callback);
	void deleteCallback(SDL_Keycode keycode);

	void processInput(SDL_Keycode keycode);

private:

	std::map<SDL_Keycode, std::function<void()>> callbacks;
};
