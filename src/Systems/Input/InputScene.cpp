#include "InputScene.h"
#include "InputObject.h"
#include "../SystemInterface.h"
#include "../../Engine/UObject.h"
#include "../../Managers/ServiceManagerInterface.h"
#include "../../UI/UIManager.h"

InputScene::InputScene(const SystemInterface* system, const UScene* uscene) :
	SystemSceneInterface(system, uscene),
	bPlacingSelectedObjects(false)
{
	uiManager = std::make_unique<UIManager>();
}

InputScene::~InputScene()
{
}

SystemObjectInterface* InputScene::addSystemObject(SystemObjectInterface* object)
{
	inputObjects.emplace_back(static_cast<InputObject*>(object));
	return object;
}

void InputScene::handleEvent(const SDL_Event& sdlEvent)
{
	UIManagerEvent uiEvent = uiManager->handeInput(sdlEvent);

	if (uiEvent.spawned) {
		for (auto* obj : selectedObjects) obj->event(EventType::SelectionUpdated, false);
		selectedObjects.clear();

		selectedObjects.push_back(uiEvent.spawned);
		uiEvent.spawned->event(EventType::SelectionUpdated, true);
		bPlacingSelectedObjects = true;
	}

	if (uiEvent.bConsumedInput) return;

	if (bPlacingSelectedObjects) {
		handlePlacingInput(sdlEvent);
	}
	else {
		handleObjectManagementInput(sdlEvent);
	}

	for (const auto& object : inputObjects) {
		object->handleEvent(sdlEvent);
	}
}

void InputScene::handlePlacingInput(const SDL_Event& sdlEvent)
{
	//if (sdlEvent.type == SDL_MOUSEMOTION) {
	//	mat::vec3 hitLoc;
	//	if (engine.raycastScreen(sdlEvent.motion.x, sdlEvent.motion.y, hitLoc, selectedObjects)) {
	//		for (auto* obj : selectedObjects) obj->uobject->event(EventType::PositionUpdated, hitLoc);
	//	}
	//}
	//else if (sdlEvent.type == SDL_MOUSEBUTTONDOWN && sdlEvent.button.button == SDL_BUTTON_LEFT) {
	//	for (auto* obj : selectedObjects) obj->uobject->event(EventType::SelectionUpdated, true);
	//	bPlacingSelectedObjects = false;
	//}
	//else if (sdlEvent.type == SDL_KEYDOWN && sdlEvent.key.keysym.sym == SDLK_DELETE) {
	//	for (auto* obj : selectedObjects) engine.world().destroyObject(obj);
	//	selectedObjects.clear();
	//	bPlacingSelectedObjects = false;
	//}
}

void InputScene::handleObjectManagementInput(const SDL_Event& sdlEvent)
{
	// Object selection
	if (sdlEvent.type == SDL_MOUSEBUTTONDOWN && sdlEvent.button.button == SDL_BUTTON_LEFT) {
		if (const UObject* hitObject = system->serviceManager->raycastScreen(uscene, sdlEvent.motion.x, sdlEvent.motion.y)) {
			// clicked on object
			for (auto* obj : selectedObjects) obj->event(EventType::SelectionUpdated, false);
			selectedObjects.clear();

			hitObject->event(EventType::SelectionUpdated, true);
			selectedObjects.push_back(hitObject);
			//auto* uiComp = hitObject->uiComponent();
			//uiManager->setActiveData(uiComp ? &uiComp->data : nullptr);
		}
		else {
			// clicked empty space
			for (auto* obj : selectedObjects) obj->event(EventType::SelectionUpdated, false);
			selectedObjects.clear();
			uiManager->setActiveData(nullptr);
		}
	}
	// Object deletion
	else if (sdlEvent.type == SDL_KEYDOWN && sdlEvent.key.keysym.sym == SDLK_DELETE) {
		// for (auto* obj : selectedObjects) engine.world().destroyObject(obj);
		selectedObjects.clear();
	}
}

void InputScene::tick(float deltaTime)
{
	for (const auto& object : inputObjects) {
		object->tick(deltaTime);
	}
}
