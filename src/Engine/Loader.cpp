#include "Loader.h"
#include "UScene.h"

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

Loader::Loader() :
	inputSystem(nullptr),
	graphicsSystem(nullptr)
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

	// return all system interfaces
	return SystemsWrapper{
	std::move(input),
	std::move(graphics),
	std::move(physics)
	};
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
	auto* inputScene = inputSystem->findSystemScene(uscene);
	auto* graphicsScene = graphicsSystem->findSystemScene(uscene);

	UObject* uobject = uscene->createUniversalObject();
	if (asset.assetType == AssetType::Object && !asset.modelPath.empty()) {
		auto* gobj = graphicsScene->createSystemObject<MeshGraphicsObject>(uobject);
		gobj->setMesh(asset.modelPath);
	}
	return uobject;
}
