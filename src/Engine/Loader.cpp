#include "Loader.h"
#include "UScene.h"
#include "UObject.h"

#include "../Managers/AssetManager.h"
#include "../Managers/ServiceManager.h"

#include "../Systems/SystemInterface.h"
#include "../Systems/SystemSceneInterface.h"

#include "../Systems/Input/InputSystem.h"
#include "../Systems/Input/CameraInputObject.h"

#include "../Systems/Graphics/GraphicsSystem.h"
#include "../Systems/Graphics/GraphicsScene.h"
#include "../Systems/Graphics/CameraGraphicsObject.h"
#include "../Systems/Graphics/MeshGraphicsObject.h"
#include "../Systems/Graphics/UIGraphicsObject.h"

#include "../Systems/Physics/PhysicsSystem.h"
#include "../Systems/Physics/PhysicsObject.h"

#include "../Systems/Audio/AudioSystem.h"
#include "../Systems/Audio/AudioScene.h"
#include "../Systems/Audio/AudioObject.h"
#include "../Systems/Audio/Components/AMicrophone.h"
#include "../Systems/Audio/Components/ASpeaker.h"

Loader::Loader() :
	inputSystem(nullptr),
	graphicsSystem(nullptr),
	physicsSystem(nullptr),
	audioSystem(nullptr)
{
}

Loader::SystemsWrapper Loader::createSystems()
{
	// create systems
	auto input    = std::make_unique<InputSystem>();
	auto graphics = std::make_unique<GraphicsSystem>();
	auto physics  = std::make_unique<PhysicsSystem>();
	auto audio    = std::make_unique<AudioSystem>();

	// register system services with ServiceManager
	auto& serviceManager = ServiceManager::instance();
	serviceManager.graphicsSystem = graphics.get();
	serviceManager.physicsSystem  = physics.get();

	// register manager interfaces with systems
	auto& assetManager = AssetManager::instance();
	input->assetManager      = &assetManager;
	input->serviceManager    = &serviceManager;
	graphics->assetManager   = &assetManager;
	graphics->serviceManager = &serviceManager;
	physics->assetManager    = &assetManager;
	physics->serviceManager  = &serviceManager;
	audio->assetManager      = &assetManager;
	audio->serviceManager    = &serviceManager;

	// save pointers for use inside Loader
	inputSystem    = input.get();
	graphicsSystem = graphics.get();
	physicsSystem  = physics.get();
	audioSystem    = audio.get();

	// return all system interfaces
	return SystemsWrapper{
	std::move(input),
	std::move(graphics),
	std::move(physics),
	std::move(audio)
	};
}

void Loader::loadDefaultScene(UScene* uscene)
{
	inputSystem->createSystemScene(uscene);
	graphicsSystem->createSystemScene(uscene);
	physicsSystem->createSystemScene(uscene);
	audioSystem->createSystemScene(uscene);

	createDefaultCamera(uscene);

	AssetDescriptor asset;
	if (AssetManager::instance().descriptor("Platform", asset)) {
		for (int x = -2; x <= 2; x++) {
			for (int z = -1; z <= 2; z++) {
				auto* uplatform = createObjectFromAsset(asset, uscene);
				uplatform->eventImmediate(
					EventType::PositionUpdated,
					mat::vec3 { static_cast<float>(x), 0, static_cast<float>(z) - 0.5f });
			}
		}
	}
}

UObject* Loader::createDefaultCamera(UScene* uscene)
{
	auto* inputScene = inputSystem->findSystemScene(uscene);
	auto* graphicsScene = graphicsSystem->findSystemScene(uscene);

	UObject* ucamera = uscene->createUniversalObject();
	inputScene->createSystemObject<CameraInputObject>(ucamera);
	auto* gobj = graphicsScene->createSystemObject<CameraGraphicsObject>(ucamera);
	static_cast<GraphicsScene*>(graphicsScene)->activeCamera = gobj;
	return ucamera;
}

UObject* Loader::createObjectFromAsset(AssetID asset, UScene* uscene) const
{
	AssetDescriptor descriptor;
	if (AssetManager::instance().descriptor(asset, descriptor)) {
		return createObjectFromAsset(descriptor, uscene);
	}
	return nullptr;
}

UObject* Loader::createObjectFromAsset(const AssetDescriptor& asset, UScene* uscene) const
{
	auto* graphicsScene = graphicsSystem->findSystemScene(uscene);
	auto* physicsScene = physicsSystem->findSystemScene(uscene);

	UObject* uobject = uscene->createUniversalObject();
	if (asset.assetType == AssetType::Object && !asset.modelPath.empty()) {
		auto* gobj = graphicsScene->createSystemObject<MeshGraphicsObject>(uobject);
		auto* pobj = physicsScene->createSystemObject<PhysicsObject>(uobject);
		gobj->setMesh(asset.modelPath);
		pobj->setPhysicsMesh(asset.modelPath);
	}
	if (asset.audioType != AudioType::None) {
		auto* audioScene = static_cast<AudioScene*>(audioSystem->findSystemScene(uscene));
		auto* aobj = audioScene->createSystemObject<AudioObject>(uobject);
		switch (asset.audioType) {
		case AudioType::Microphone:
			audioScene->setAudioComponentForObject<AMicrophone>(aobj);
			break;
		case AudioType::Speaker:
			audioScene->setAudioComponentForObject<ASpeaker>(aobj);
			break;
		default:
			break;
		}
	}
	return uobject;
}

UObject* Loader::createUIObject(UScene* uscene) const
{
	auto* graphicsScene = graphicsSystem->findSystemScene(uscene);

	UObject* uobject = uscene->createUniversalObject();
	graphicsScene->createSystemObject<UIGraphicsObject>(uobject);
	return uobject;
}

void Loader::deleteObject(const UObject* uobject, UScene* uscene) const
{
	inputSystem->findSystemScene(uscene)->deleteSystemObject(uobject);
	graphicsSystem->findSystemScene(uscene)->deleteSystemObject(uobject);
	physicsSystem->findSystemScene(uscene)->deleteSystemObject(uobject);
	audioSystem->findSystemScene(uscene)->deleteSystemObject(uobject);
	uscene->deleteUniversalObject(uobject);
}
