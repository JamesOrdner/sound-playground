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
		//int x, y;
		//SDL_GetMouseState(&x, &y);
		//UIObject* obj = objectAt(
		//	UICoord{
		//		static_cast<float>(x - screenWidth / 2) / (screenWidth / 2),
		//		static_cast<float>(screenHeight / 2 - y) / (screenHeight / 2)
		//	}
		//);
		//if (obj) {
		//	if (obj->callback) obj->callback();
		//	return true;
		//}
	}

	return false;
}

void UIManager::setupMenuBar()
{
	root = std::make_unique<UIObject>();
	root->anchor = UIAnchor::Bottom;
	root->position = mat::vec2{ 0.f, -1.f };
	root->bounds = mat::vec2{ 512, 512 };
	root->bAcceptsInput = true;
	root->callback = [] { printf("Big button pressed.\n"); };

	UIObject& child = root->subobjects.emplace_back();
	child.bounds = mat::vec2{ 90, 90 };
	child.bAcceptsInput = true;
	child.callback = [] { printf("Little button pressed.\n"); };
}

UIObject* UIManager::objectAt(const UICoord& location)
{
	return nullptr;
	// return objectAtRecursive(*root, location, UICoord{ 0.f, 0.f }, UICoord{ 1.f, 1.f });
}

UIObject* UIManager::objectAtRecursive(
	UIObject& object,
	const UICoord& location,
	const UICoord& p_trans,
	const UICoord& p_scale)
{
	//UICoord scale{
	//	(object.x1 - object.x0) / 2.f,
	//	(object.y1 - object.y0) / 2.f
	//};

	//UICoord trans{
	//	object.x0 + scale.x + p_trans.x,
	//	object.y0 + scale.y + p_trans.y
	//};

	//scale.x *= p_scale.x;
	//scale.y *= p_scale.y;

	//for (auto it = object.subobjects.rbegin(); it != object.subobjects.rend(); it++) {
	//	if (UIObject* obj = objectAtRecursive(*it, location, trans, scale)) return obj;
	//}

	//if (!object.bAcceptsInput) return nullptr;
	//if (location.x < -scale.x + trans.x) return nullptr;
	//if (location.y < -scale.y + trans.y) return nullptr;
	//if (location.x > scale.x + trans.x) return nullptr;
	//if (location.y > scale.y + trans.y) return nullptr;
	//return &object;

	return nullptr;
}
