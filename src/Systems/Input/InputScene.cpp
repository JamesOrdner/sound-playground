#include "InputScene.h"
#include "InputObject.h"
#include "../SystemInterface.h"
#include "../../Engine/UScene.h"
#include "../../Engine/UObject.h"
#include "../../Managers/ServiceManagerInterface.h"
#include "../../UI/UIManager.h"
#include "../../UI/UIObject.h"

InputScene::InputScene(const SystemInterface* system, const UScene* uscene) :
	SystemSceneInterface(system, uscene),
	bPlacingSelectedObjects(false)
{
	uiManager = std::make_unique<UIManager>(system->assetManager);
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
	handeUIManagerEvent(uiEvent);

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

void InputScene::handeUIManagerEvent(const UIManagerEvent& event)
{
	if (event.type == UIManagerEvent::Type::SpawnObject) {
		for (auto* obj : selectedObjects) obj->event(EventType::SelectionUpdated, false);
		selectedObjects.clear();

		CreateObjectRequestData spawnRequest{
			event.spawnID,
			this,
			[](UObject* object, void* userData) { static_cast<InputScene*>(userData)->objectCreated(object); }
		};
		uscene->event(EventType::CreateObjectRequest, spawnRequest);
	}
}

void InputScene::objectCreated(UObject* object)
{
	selectedObjects.emplace(object);
	bPlacingSelectedObjects = true;
}

void InputScene::handlePlacingInput(const SDL_Event& sdlEvent)
{
	if (sdlEvent.type == SDL_MOUSEMOTION) {
		mat::vec3 hitLoc;
		if (system->serviceManager->raycastScreen(uscene, sdlEvent.motion.x, sdlEvent.motion.y, hitLoc, selectedObjects)) {
			for (auto* obj : selectedObjects) obj->event(EventType::PositionUpdated, hitLoc);
		}
	}
	else if (sdlEvent.type == SDL_MOUSEBUTTONDOWN && sdlEvent.button.button == SDL_BUTTON_LEFT) {
		for (auto* obj : selectedObjects) obj->event(EventType::SelectionUpdated, true);
		bPlacingSelectedObjects = false;
	}
	else if (sdlEvent.type == SDL_KEYDOWN && sdlEvent.key.keysym.sym == SDLK_DELETE) {
		// for (auto* obj : selectedObjects) engine.world().destroyObject(obj);
		selectedObjects.clear();
		bPlacingSelectedObjects = false;
	}
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
			selectedObjects.emplace(hitObject);
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
	processUIManagerQueue();

	for (const auto& object : inputObjects) {
		object->tick(deltaTime);
	}
}

void InputScene::processUIManagerQueue()
{
	while (!uiManager->modificationQueue.empty()) {
		auto& modEvent = uiManager->modificationQueue.front();
		auto* uobject = static_cast<UObject*>(modEvent.object->userData);
		switch (modEvent.modification) {
		case UIObjectModification::Type::ObjectCreated:
			uscene->event(
				EventType::CreateUIObjectRequest,
				CreateObjectRequestData {
					AssetID(),
					this,
					[](UObject* object, void* userData) { static_cast<InputScene*>(userData)->uiObjectCreated(object); }
				}
			);
			break;
		case UIObjectModification::Type::TexturePositionUpdated:
			if (uobject) uobject->event(EventType::UITexturePositionUpdated, modEvent.object->replicatedData.texturePosition);
			break;
		}
		uiManager->modificationQueue.pop();
	}
}

void InputScene::uiObjectCreated(UObject* object)
{
	// Probably want to optimize this later, but for now just iterate all UIObjects to find unassigned one
	UIObject* assigned = assignUObjectToUIObject(object, uiManager->root.get());
	uiObjectPool[object] = assigned;

	if (!assigned) return;

	// Set properties. All events immediate, as uiObjectCreated() is assumed to be run synchronously
	object->eventImmediate(EventType::UIPositionUpdated, assigned->replicatedData.position);
	object->eventImmediate(EventType::UIBoundsUpdated, assigned->replicatedData.bounds);
	object->eventImmediate(EventType::UIDrawOrderUpdated, assigned->replicatedData.drawOrder);
	object->eventImmediate(EventType::UITextureAssetUpdated, assigned->replicatedData.textureAsset);
	object->eventImmediate(EventType::UITexturePositionUpdated, assigned->replicatedData.texturePosition);
	object->eventImmediate(EventType::UITextureBoundsUpdated, assigned->replicatedData.textureBounds);
}

UIObject* InputScene::assignUObjectToUIObject(UObject* uobject, UIObject* root)
{
	if (!root->userData) {
		root->userData = uobject;
		return root;
	}

	for (auto& subobject : root->subobjects) {
		if (auto* assigned = assignUObjectToUIObject(uobject, &subobject)) return assigned;
	}

	return nullptr;
}
