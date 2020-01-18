#include "UScene.h"
#include "UObject.h"
#include "Engine.h"
#include "LoaderInterface.h"

UScene::UScene(const Engine* engine) :
	engine(engine)
{
	registerCallback(
		this,
		EventType::CreateObjectRequest,
		[this](const EventData& data) {
			auto createObjectRequest = std::get<CreateObjectRequestData>(data);
			auto* uobject = this->engine->loaderInterface()->createObjectFromAsset(createObjectRequest.assetID, this);
			if (createObjectRequest.callback) createObjectRequest.callback(uobject);
		}
	);
}

UScene::~UScene()
{
}

UObject* UScene::createUniversalObject()
{
	return uobjects.emplace_back(std::make_unique<UObject>()).get();
}
