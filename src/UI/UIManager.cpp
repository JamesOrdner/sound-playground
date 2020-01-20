#include "UIManager.h"
#include "UIObject.h"
#include "../Managers/AssetManagerInterface.h"

mat::vec2 UIManager::screenBounds = mat::vec2{ 1280, 720 };

UIManager::UIManager(const AssetManagerInterface* assetManager) :
	assetManager(assetManager),
	hoveredObject(nullptr),
	propertiesData(nullptr)
{
	root = std::make_unique<UIObject>();
	root->bounds = screenBounds;
	root->replicatedData.bounds = screenBounds;
	modificationQueue.push(
		UIObjectModification{ UIObjectModification::Type::ObjectCreated, root.get() }
	);
	
	setupMenuBar();
	setupProperties();
	updateAllReplicatedData();
}

UIManager::~UIManager()
{
}

void UIManager::setupMenuBar()
{
	UIObject& menuBar = createUIObject(*root);
	menuBar.anchor = UIAnchor::Bottom;
	menuBar.bounds = mat::vec2{ 768, 100 };
	menuBar.bAcceptsInput = true;
	menuBar.textureCoords = []() { return mat::vec4{ 0, 0, 768, 100 }; };

	UIObject& speakerButton = createUIObject(menuBar);
	speakerButton.anchor = UIAnchor::Left;
	speakerButton.bounds = mat::vec2{ 80, 80 };
	speakerButton.position = mat::vec2{ 10, 0 };
	speakerButton.bAcceptsInput = true;
	AssetID speakerID;
	if (assetManager->assetID("Speaker", speakerID)) {
		speakerButton.callback = [speakerID](UIManagerEvent& uiEvent) {
			uiEvent.type = UIManagerEvent::Type::SpawnObject;
			uiEvent.spawnID = speakerID;
		};
	}
	speakerButton.textureCoords = [&state = speakerButton.state]() {
		switch (state) {
		case UIObjectState::Hovered:  return mat::vec4{ 81, 101, 80, 80 };
		default:                      return mat::vec4{  0, 101, 80, 80 };
		}
	};

	UIObject& speaker = createUIObject(speakerButton);
	speaker.anchor = UIAnchor::Center;
	speaker.bounds = mat::vec2{ 60, 60 };
	speaker.textureCoords = []() { return mat::vec4{ 162, 101, 60, 60 }; };
}

void UIManager::setupProperties()
{
	UIObject& propertiesPanel = createUIObject(*root);
	propertiesPanel.anchor = UIAnchor::BottomRight;
	propertiesPanel.bounds = mat::vec2{ 256, 256 };
	propertiesPanel.bAcceptsInput = true;
	propertiesPanel.textureCoords = []() { return mat::vec4{ 768, 0, 256, 256 }; };
	propertiesPanel.setAnimationTarget(mat::vec2{ 0, -256 });
	propertiesPanel.animationRate = 15.f;
	propertiesRoot = &propertiesPanel;
}

UIObject& UIManager::createUIObject(UIObject& parent)
{
	UIObject& newObject = parent.subobjects.emplace_back();
	modificationQueue.push(
		UIObjectModification{
			UIObjectModification::Type::ObjectCreated,
			&newObject
		}
	);
	return newObject;
}

void UIManager::updateAllReplicatedData()
{
	uint32_t rootDrawIndex = 0;
	assignDrawOrders(*root, rootDrawIndex);

	updateAllReplicatedData(*root, *root);
}

void UIManager::updateAllReplicatedData(UIObject& object, UIObject& parent)
{
	mat::vec2 anchorOffset = (parent.replicatedData.bounds - object.bounds) / 2.f * object.anchorPosition();
	mat::vec2 center = parent.replicatedData.position + parent.replicatedData.bounds / 2.f + anchorOffset + object.position; // virtual pixels
	mat::vec2 position = center - object.bounds / 2.f;
	if (object.replicatedData.position != position) {
		object.replicatedData.position = position;
		modificationQueue.push(UIObjectModification{ UIObjectModification::Type::PositionUpdated, &object });
	}

	if (object.replicatedData.bounds != object.bounds) {
		object.replicatedData.bounds = object.bounds;
		modificationQueue.push(UIObjectModification{ UIObjectModification::Type::BoundsUpdated, &object });
	}

	mat::vec4 texCoords = object.textureCoords();
	mat::vec2 texturePosition{ texCoords[0], texCoords[1] };
	mat::vec2 textureBounds{ texCoords[2], texCoords[3] };

	if (object.replicatedData.texturePosition != texturePosition) {
		object.replicatedData.texturePosition = texturePosition;
		modificationQueue.push(UIObjectModification{ UIObjectModification::Type::TexturePositionUpdated, &object });
	}

	if (object.replicatedData.textureBounds != textureBounds) {
		object.replicatedData.textureBounds = textureBounds;
		modificationQueue.push(UIObjectModification{ UIObjectModification::Type::TextureBoundsUpdated, &object });
	}

	for (auto& subobject : object.subobjects) {
		updateAllReplicatedData(subobject, object);
	}
}

void UIManager::assignDrawOrders(UIObject& rootObject, uint32_t& currentOrderIndex)
{
	if (rootObject.replicatedData.drawOrder != currentOrderIndex) {
		rootObject.replicatedData.drawOrder = currentOrderIndex;
		modificationQueue.push(UIObjectModification{ UIObjectModification::Type::DrawOrderUpdated, &rootObject });
	}
	for (auto& subobject : rootObject.subobjects) assignDrawOrders(subobject, ++currentOrderIndex);
}

UIManagerEvent UIManager::handeInput(const SDL_Event& event)
{
	UIManagerEvent uiEvent = {};
	if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT) {
		if (UIObject* obj = objectAt(virtualMousePosition(event))) {
			if (obj->callback) obj->callback(uiEvent);
			uiEvent.bConsumedInput = true;
			return uiEvent;
		}
	}
	else if (event.type == SDL_MOUSEMOTION) {
		UIObject* obj = objectAt(virtualMousePosition(event));
		if (obj != hoveredObject) {
			if (hoveredObject && hoveredObject->state == UIObjectState::Hovered) {
				hoveredObject->state = UIObjectState::Neutral;
				updateAllReplicatedData();
			}

			if (obj) {
				obj->state = UIObjectState::Hovered;
				updateAllReplicatedData();
			}
			hoveredObject = obj;
		}
	}

	return uiEvent;
}

void UIManager::setActiveData(std::vector<UIData>* data)
{
	propertiesData = data;
	if (propertiesData) {
		propertiesRoot->setAnimationTarget(mat::vec2{ 0, 0 });
	}
	else {
		propertiesRoot->setAnimationTarget(mat::vec2{ 0, -256 });
	}
}

void UIManager::tick(float deltaTime)
{
	root->tick(deltaTime);
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

	for (auto& subobject : object.subobjects) {
		UIObject* obj = objectAtRecursive(subobject, location, center, object.bounds);
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
