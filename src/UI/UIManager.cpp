#include "UIManager.h"
#include "UIObject.h"
#include "../Engine/Engine.h"
#include "../Engine/EWorld.h"
#include "../Engine/EModel.h"
#include <SDL_events.h>

mat::vec2 UIManager::screenBounds = mat::vec2{ 1280, 720 };

UIManager::UIManager() :
	hoveredObject(nullptr)
{
	root = std::make_unique<UIObject>();
	root->bounds = screenBounds;
	
	setupMenuBar();
}

UIManager::~UIManager()
{
}

UIManagerEvent UIManager::handeInput(const SDL_Event& event, SDL_Window* window)
{
	UIManagerEvent uiEvent = {};
	if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT) {
		if (UIObject* obj = objectAt(virtualMousePosition(window))) {
			if (obj->callback) obj->callback(uiEvent);
			uiEvent.bConsumedInput = true;
			return uiEvent;
		}
	}
	else if (event.type == SDL_MOUSEMOTION) {
		UIObject* obj = objectAt(virtualMousePosition(window));
		if (obj != hoveredObject) {
			if (hoveredObject && hoveredObject->state == UIObjectState::Hovered) {
				hoveredObject->state = UIObjectState::Neutral;
			}

			if (obj) obj->state = UIObjectState::Hovered;
			hoveredObject = obj;
		}
	}

	return uiEvent;
}

mat::vec2 UIManager::virtualMousePosition(SDL_Window* window)
{
	int x, y, width, height;
	SDL_GetMouseState(&x, &y);
	SDL_GL_GetDrawableSize(window, &width, &height);
	return mat::vec2{ static_cast<float>(x) / width, 1.f - static_cast<float>(y) / height } * screenBounds;
}

void UIManager::setupMenuBar()
{
	UIObject& menuBar = root->subobjects.emplace_back();
	menuBar.anchor = UIAnchor::Bottom;
	menuBar.bounds = mat::vec2{ 768, 100 };
	menuBar.bAcceptsInput = true;
	menuBar.textureCoords = []() { return mat::vec4{ 0, 0, 768, 100 }; };

	UIObject& speakerButton = menuBar.subobjects.emplace_back();
	speakerButton.anchor = UIAnchor::Left;
	speakerButton.bounds = mat::vec2{ 80, 80 };
	speakerButton.position = mat::vec2{ 10, 0 };
	speakerButton.bAcceptsInput = true;
	speakerButton.callback = [](UIManagerEvent& uiEvent) {
		EModel* speaker = Engine::instance().world().spawnObject<EModel>();
		speaker->setMesh("res/speaker_small.glb");
		speaker->setRotation(mat::vec3{ 0, mat::pi, 0 });
		uiEvent.spawned = speaker;
	};
	speakerButton.textureCoords = [&state = speakerButton.state]() {
		switch (state) {
		case UIObjectState::Hovered:  return mat::vec4{ 81, 101, 80, 80 };
		default:                      return mat::vec4{  0, 101, 80, 80 };
		}
	};

	UIObject& speaker = speakerButton.subobjects.emplace_back();
	speaker.anchor = UIAnchor::Center;
	speaker.bounds = mat::vec2{ 60, 60 };
	speaker.textureCoords = []() { return mat::vec4{ 162, 101, 60, 60 }; };
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
