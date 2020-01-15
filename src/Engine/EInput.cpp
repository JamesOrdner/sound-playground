#include "EInput.h"
#include "Engine.h"
#include "EInputComponent.h"
#include "../UI/UITypes.h"
#include "../UI/UIManager.h"
#include "../UI/UIComponent.h"

EInput::EInput() :
	uiManager(nullptr),
	bPlacingSelected(false)
{
}

void EInput::handleInput(const SDL_Event& sdlEvent)
{
	//UIManagerEvent uiEvent = uiManager->handeInput(sdlEvent);

	//if (uiEvent.spawned) {
	//	for (auto* obj : selectedObjects) obj->setSelected(false);
	//	selectedObjects.clear();
	//	selectedObjects.emplace(uiEvent.spawned);
	//	bPlacingSelected = true;
	//}

	//if (uiEvent.bConsumedInput) return;

	//if (bPlacingSelected) {
	//	handleInputPlacing(sdlEvent);
	//}
	//else {
	//	handleInputObjectManagement(sdlEvent);
	//}

	//// Forward to object input components
 //   for (const auto& object : Engine::instance().world().allObjects()) {
 //       if (auto* inputComponent = object->inputComponent()) {
 //           inputComponent->processInput(sdlEvent);
 //       }
 //   }
}

void EInput::handleInputPlacing(const SDL_Event& sdlEvent)
{
	//auto& engine = Engine::instance();

	//if (sdlEvent.type == SDL_MOUSEMOTION) {
	//	mat::vec3 hitLoc;
	//	if (EModel* model = engine.raycastScreen(sdlEvent.motion.x, sdlEvent.motion.y, hitLoc, selectedObjects)) {
	//		for (auto* obj : selectedObjects) obj->setPosition(hitLoc);
	//	}
	//}
	//else if (sdlEvent.type == SDL_MOUSEBUTTONDOWN && sdlEvent.button.button == SDL_BUTTON_LEFT) {
	//	bPlacingSelected = false;
	//	for (auto* obj : selectedObjects) obj->setSelected(true);
	//}
	//else if (sdlEvent.type == SDL_KEYDOWN && sdlEvent.key.keysym.sym == SDLK_ESCAPE) {
	//	for (auto* obj : selectedObjects) engine.world().destroyObject(obj);
	//	selectedObjects.clear();
	//	bPlacingSelected = false;
	//}
}

void EInput::handleInputObjectManagement(const SDL_Event& sdlEvent)
{
	//auto& engine = Engine::instance();

	//// Object selection
	//if (sdlEvent.type == SDL_MOUSEBUTTONDOWN && sdlEvent.button.button == SDL_BUTTON_LEFT) {
	//	if (EModel* model = engine.raycastScreen(sdlEvent.motion.x, sdlEvent.motion.y)) {
	//		// clicked on object
	//		for (auto* obj : selectedObjects) obj->setSelected(false);
	//		selectedObjects.clear();

	//		model->setSelected(true);
	//		selectedObjects.emplace(model);
	//		auto* uiComp = model->uiComponent();
	//		uiManager->setActiveData(uiComp ? &uiComp->data : nullptr);
	//	}
	//	else {
	//		// clicked empty space
	//		for (auto* obj : selectedObjects) obj->setSelected(false);
	//		selectedObjects.clear();
	//		uiManager->setActiveData(nullptr);
	//	}
	//}
	//// Object deletion
	//else if (sdlEvent.type == SDL_KEYDOWN && sdlEvent.key.keysym.sym == SDLK_DELETE) {
	//	for (auto* obj : selectedObjects) engine.world().destroyObject(obj);
	//	selectedObjects.clear();
	//}
}
