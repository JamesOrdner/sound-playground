#include "UIManager.h"
#include "UIObject.h"
#include <SDL_events.h>

mat::vec2 UIManager::screenBounds = mat::vec2{ 1280, 720 };

UIManager::UIManager()
{
	root = std::make_unique<UIObject>();
	root->bounds = screenBounds;

	setupMenuBar();
}

UIManager::~UIManager()
{
}

bool UIManager::handeInput(const SDL_Event& event, SDL_Window* window)
{
	if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT) {
		int x, y, width, height;
		SDL_GetMouseState(&x, &y);
		SDL_GL_GetDrawableSize(window, &width, &height);
		mat::vec2 nLoc{ static_cast<float>(x) / width, 1.f - static_cast<float>(y) / height };

		UIObject* obj = objectAt(nLoc * screenBounds);
		if (obj) {
			if (obj->callback) obj->callback();
			return true;
		}
	}

	return false;
}

void UIManager::setupMenuBar()
{
	UIObject& menuBar = root->subobjects.emplace_back();
	menuBar.anchor = UIAnchor::Bottom;
	// menuBar.position = mat::vec2{ 0.f, -1.f };
	menuBar.bounds = mat::vec2{ 768, 100 };
	menuBar.textureCoords = mat::vec4{ 0, 924, 768, 1024 };
	menuBar.bAcceptsInput = true;
	menuBar.callback = [] { printf("Big button pressed.\n"); };

	UIObject& child = menuBar.subobjects.emplace_back();
	child.anchor = UIAnchor::Left;
	child.bounds = mat::vec2{ 80, 80 };
	child.position = mat::vec2{ 10, 0 };
	child.textureCoords = mat::vec4{ 0, 843, 80, 923 };
	child.bAcceptsInput = true;
	child.callback = [] { printf("Little button pressed.\n"); };
}

UIObject* UIManager::objectAt(const mat::vec2& location)
{
	return objectAtRecursive(*root, location, screenBounds / 2.f, screenBounds);
}

UIObject* UIManager::objectAtRecursive(
	UIObject& object,
	const mat::vec2& location,
	const mat::vec2& parentCenterAbs,
	const mat::vec2& parentBoundsAbs)
{
	mat::vec2 anchorOffset = (parentBoundsAbs - object.bounds) / 2.f * object.anchorPosition();
	mat::vec2 center = parentCenterAbs + anchorOffset + object.position; // virtual pixels

	for (auto it = object.subobjects.rbegin(); it != object.subobjects.rend(); it++) {
		UIObject* obj = objectAtRecursive(*it, location, center, object.bounds);
		if (obj) return obj;
	}

	mat::vec2 halfBounds = object.bounds / 2.f;
	if (!object.bAcceptsInput) return nullptr;
	if (location.x < center.x - halfBounds.x) return nullptr;
	if (location.y < center.y - halfBounds.y) return nullptr;
	if (location.x > center.x + halfBounds.x) return nullptr;
	if (location.y > center.y + halfBounds.y) return nullptr;
	return &object;
}
