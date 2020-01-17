#include "Loader.h"
#include "UScene.h"
#include "../Managers/AssetManager.h"
#include "SystemInterface.h"
#include "SystemSceneInterface.h"
#include "../Graphics/GraphicsScene.h"
#include "../Graphics/CameraGraphicsObject.h"
#include "../Graphics/MeshGraphicsObject.h"
#include "../Input/CameraInputObject.h"

Loader::Loader(
	const AssetManager* assetManager,
	SystemInterface* inputSystem,
	SystemInterface* graphicsSystem) :
	assetManager(assetManager),
	inputSystem(inputSystem),
	graphicsSystem(graphicsSystem)
{
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
	if (assetManager->descriptor(asset, descriptor)) {
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
