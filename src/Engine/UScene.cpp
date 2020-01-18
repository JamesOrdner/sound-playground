#include "UScene.h"
#include "UObject.h"
#include "Engine.h"
#include "LoaderInterface.h"
#include "../Managers/AssetTypes.h"

UScene::UScene(const Engine* engine) :
	engine(engine)
{
}

UScene::~UScene()
{
}

UObject* UScene::createUniversalObject()
{
	UObject* object = uobjects.emplace_back(std::make_unique<UObject>()).get();

	// register this object to allow the object to spawn new objects
	registerCallback(
		object,
		EventType::CreateObjectRequest,
		[this](const EventData& data) {
			auto createObjectRequest = std::get<CreateObjectRequestData>(data);
			auto* uobject = engine->loaderInterface()->createObjectFromAsset(createObjectRequest.assetID, this);
			if (createObjectRequest.callback) createObjectRequest.callback(uobject);
		}
	);

	return object;
}
