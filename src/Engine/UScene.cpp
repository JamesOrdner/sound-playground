#include "UScene.h"
#include "UObject.h"

UScene::~UScene()
{
}

UObject* UScene::createUniversalObject()
{
	UObject* object = objects.emplace_back(std::make_unique<UObject>()).get();

	// register this object to allow the object to spawn new objects
	registerCallback(
		object,
		EventType::CreateObjectRequest,
		[this](const EventData& data) {
			UObject::UObjectFactory create = std::get<UObject::UObjectFactory>(data);
			objects.emplace_back(create());
		}
	);

	return object;
}
