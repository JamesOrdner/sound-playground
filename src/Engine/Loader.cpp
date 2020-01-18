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

#include "../Systems/Physics/PhysicsSystem.h"
#include "../Systems/Physics/PhysicsObject.h"

Loader::Loader() :
	inputSystem(nullptr),
	graphicsSystem(nullptr),
	physicsSystem(nullptr)
{
}

Loader::SystemsWrapper Loader::createSystems()
{
	// create systems
	auto input = std::make_unique<InputSystem>();
	auto graphics = std::make_unique<GraphicsSystem>();
	auto physics = std::make_unique<PhysicsSystem>();

	// register system services with ServiceManager
	auto& serviceManager = ServiceManager::instance();
	serviceManager.physicsSystem = physics.get();

	// register manager interfaces with systems
	input->serviceManager = &serviceManager;
	graphics->serviceManager = &serviceManager;
	physics->serviceManager = &serviceManager;

	// save pointers for use inside Loader
	inputSystem = input.get();
	graphicsSystem = graphics.get();
	physicsSystem = physics.get();

	// return all system interfaces
	return SystemsWrapper{
	std::move(input),
	std::move(graphics),
	std::move(physics)
	};
}

void Loader::loadDefaultScene(UScene* uscene)
{
	inputSystem->createSystemScene(uscene);
	graphicsSystem->createSystemScene(uscene);
	physicsSystem->createSystemScene(uscene);

	createDefaultCamera(uscene);

	AssetDescriptor asset;
	if (AssetManager::instance().descriptor("Platform", asset)) {
		for (int x = -2; x <= 2; x++) {
			for (int z = -1; z <= 2; z++) {
				auto* uplatform = createObjectFromAsset(asset, uscene);
				uplatform->event(
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
	return uobject;
}
