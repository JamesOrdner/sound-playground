#include "AudioSystem.h"
#include "AudioScene.h"

AudioSystem::AudioSystem()
{
}

AudioSystem::~AudioSystem()
{
}

bool AudioSystem::init()
{
	return true;
}

void AudioSystem::deinit()
{
	audioScenes.clear();
}

void AudioSystem::execute(float deltaTime)
{

}

SystemSceneInterface* AudioSystem::createSystemScene(const class UScene* uscene)
{
	return audioScenes.emplace_back(std::make_unique<AudioScene>(this, uscene)).get();
}

SystemSceneInterface* AudioSystem::findSystemScene(const UScene* uscene)
{
	for (const auto& scene : audioScenes) {
		if (scene->uscene == uscene) return scene.get();
	}
	return nullptr;
}
