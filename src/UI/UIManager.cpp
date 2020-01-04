#include "UIManager.h"
#include "UIObject.h"
#include <SDL_events.h>

UIManager::UIManager()
{
	setupMenuBar();
}

UIManager::~UIManager()
{
}

bool UIManager::handeInput(const SDL_Event& event, int screenWidth, int screenHeight)
{
	if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT) {
		int x, y;
		SDL_GetMouseState(&x, &y);
		UIObject* obj = objectAt(
			mat::vec2{
				static_cast<float>(x - screenWidth / 2) / (screenWidth / 2),
				static_cast<float>(screenHeight / 2 - y) / (screenHeight / 2)
			},
			mat::vec2{
				static_cast<float>(screenWidth),
				static_cast<float>(screenHeight)
			}
		);
		if (obj) {
			if (obj->callback) obj->callback();
			return true;
		}
	}

	return false;
}

void UIManager::setupMenuBar()
{
	root = std::make_unique<UIObject>();
	root->anchor = UIAnchor::Bottom;
	root->position = mat::vec2{ 0.f, -1.f };
	root->bounds = mat::vec2{ 512, 128 };
	root->bAcceptsInput = true;
	root->callback = [] { printf("Big button pressed.\n"); };

	UIObject& child = root->subobjects.emplace_back();
	child.bounds = mat::vec2{ 90, 90 };
	child.bAcceptsInput = true;
	child.callback = [] { printf("Little button pressed.\n"); };
}

UIObject* UIManager::objectAt(const mat::vec2& location, const mat::vec2& screenBounds)
{
	return objectAtRecursive(*root, location, mat::vec2{ 0.f, 0.f }, 1.f, screenBounds);
}

UIObject* UIManager::objectAtRecursive(
	UIObject& object,
	const mat::vec2& location,
	const mat::vec2& parentCoords,
	float parentScale,
	const mat::vec2& screenBounds)
{
	mat::vec2 scale = object.bounds / screenBounds * parentScale * object.scale;
	mat::vec2 coords = parentCoords + object.position;
	coords -= object.anchorPosition() * scale;

	for (auto it = object.subobjects.rbegin(); it != object.subobjects.rend(); it++) {
		UIObject* obj = objectAtRecursive(*it, location, coords, parentScale * object.scale, screenBounds);
		if (obj) return obj;
	}

	if (!object.bAcceptsInput) return nullptr;
	if (location.x < -scale.x + coords.x) return nullptr;
	if (location.y < -scale.y + coords.y) return nullptr;
	if (location.x > scale.x + coords.x) return nullptr;
	if (location.y > scale.y + coords.y) return nullptr;
	return &object;
}
