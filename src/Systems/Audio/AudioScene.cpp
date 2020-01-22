#include "AudioScene.h"
#include "AudioObject.h"

AudioScene::AudioScene(const SystemInterface* system, const UScene* uscene) :
	SystemSceneInterface(system, uscene)
{
}

AudioScene::~AudioScene()
{
	audioObjects.clear();
}

void AudioScene::deleteSystemObject(const UObject* uobject)
{
	for (const auto& audioObject : audioObjects) {
		if (audioObject->uobject == uobject) {
			audioObjects.remove(audioObject);
			break;
		}
	}
}

SystemObjectInterface* AudioScene::addSystemObject(SystemObjectInterface* object)
{
	audioObjects.emplace_back(static_cast<AudioObject*>(object));
	return object;
}
