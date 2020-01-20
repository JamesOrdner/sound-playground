#pragma once

#include "../SystemSceneInterface.h"
#include <SDL_events.h>
#include <list>
#include <map>
#include <unordered_set>
#include <memory>

class InputScene : public SystemSceneInterface
{
public:

	InputScene(const class SystemInterface* system, const class UScene* uscene);

	~InputScene();

	void handleEvent(const SDL_Event& sdlEvent);

	// Called by the engine after a UObject creation request has been fulfilled
	void objectCreated(class UObject* object);

	// Called by the engine after a UIObject creation request has been fulfilled
	void uiObjectCreated(class UObject* object);

	void tick(float deltaTime);

private:

	std::list<std::unique_ptr<class InputObject>> inputObjects;

	SystemObjectInterface* addSystemObject(SystemObjectInterface* object) override;

	// User Interaction (non UI)

	std::unordered_set<const class UObject*> selectedObjects;

	bool bPlacingSelectedObjects;

	void handlePlacingInput(const SDL_Event& sdlEvent);
	void handleObjectManagementInput(const SDL_Event& sdlEvent);

	// UI

	std::unique_ptr<class UIManager> uiManager;

	// This map associates UObjects with UIObjects in UIManager
	std::map<class UObject*, struct UIObject*> uiObjectPool;

	void handeUIManagerEvent(const struct UIManagerEvent& event);

	// Handle all pending events in UIManager's queue
	void processUIManagerQueue();

	// Find unassigned UIObject and assign the UObject to mirror the UIObject.
	// Returns assigned UIObject on success, nullptr if all UIObjects are already assigned.
	struct UIObject* assignUObjectToUIObject(class UObject* uobject, struct UIObject* root);
};
