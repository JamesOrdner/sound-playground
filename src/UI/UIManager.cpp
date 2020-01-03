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
			ScreenCoord{
				static_cast<float>(x - screenWidth / 2) / (screenWidth / 2),
				static_cast<float>(screenHeight / 2 - y) / (screenHeight / 2)
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
	root->x0 = -0.5f;
	root->y0 = -1.0f;
	root->x1 =  0.5f;
	root->y1 = -0.7f;
	root->bAcceptsInput = true;
	root->callback = [] { printf("Big button pressed.\n"); };

	UIObject& child = root->subobjects.emplace_back();
	child.x0 = -0.5f;
	child.y0 = -1.0f;
	child.x1 =  0.5f;
	child.y1 =  1.0f;
	child.bAcceptsInput = true;
	child.callback = [] { printf("Little button pressed.\n"); };
}

UIObject* UIManager::objectAt(ScreenCoord location)
{
	return objectAtRecursive(*root, location, ScreenCoord{ 0.f, 0.f }, ScreenCoord{ 1.f, 1.f });
}

UIObject* UIManager::objectAtRecursive(
	UIObject& object,
	const ScreenCoord& location,
	ScreenCoord p_trans,
	ScreenCoord p_scale)
{
	ScreenCoord scale{
		(object.x1 - object.x0) / 2.f,
		(object.y1 - object.y0) / 2.f
	};

	ScreenCoord trans{
		object.x0 + scale.x + p_trans.x,
		object.y0 + scale.y + p_trans.y
	};

	scale.x *= p_scale.x;
	scale.y *= p_scale.y;

	for (auto it = object.subobjects.rbegin(); it != object.subobjects.rend(); it++) {
		if (UIObject* obj = objectAtRecursive(*it, location, trans, scale)) return obj;
	}

	if (!object.bAcceptsInput) return nullptr;
	if (location.x < -scale.x + trans.x) return nullptr;
	if (location.y < -scale.y + trans.y) return nullptr;
	if (location.x > scale.x + trans.x) return nullptr;
	if (location.y > scale.y + trans.y) return nullptr;
	return &object;
}
