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
		[this](const EventData& data, bool bEventFromParent) {
			auto createObjectRequest = std::get<CreateObjectRequestData>(data);
			auto* uobject = this->engine->loaderInterface()->createObjectFromAsset(createObjectRequest.assetID, this);
			if (createObjectRequest.callback) createObjectRequest.callback(uobject, createObjectRequest.userData);
		}
	);

	registerCallback(
		this,
		EventType::CreateUIObjectRequest,
		[this](const EventData& data, bool bEventFromParent) {
			auto createObjectRequest = std::get<CreateObjectRequestData>(data);
			auto* uobject = this->engine->loaderInterface()->createUIObject(this);
			if (createObjectRequest.callback) createObjectRequest.callback(uobject, createObjectRequest.userData);
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
