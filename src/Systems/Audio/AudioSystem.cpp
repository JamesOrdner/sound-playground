#include "AudioSystem.h"
#include "AudioScene.h"
#include "AudioEngine.h"

AudioSystem::AudioSystem()
{
}

AudioSystem::~AudioSystem()
{
}

bool AudioSystem::init()
{
	audioEngine = std::make_unique<AudioEngine>();
	audioEngine->init();
	return audioEngine->start();
}

void AudioSystem::deinit()
{
	for (auto& scene : audioScenes) audioEngine->unregisterScene(scene.get());
	audioScenes.clear();
	audioEngine->deinit();
	audioEngine.reset();
}

void AudioSystem::execute(float deltaTime)
{
	audioEngine->tick(deltaTime);
}

SystemSceneInterface* AudioSystem::createSystemScene(const class UScene* uscene)
{
	auto audioScene = std::make_shared<AudioScene>(this, audioEngine.get(), uscene);
	audioScenes.push_back(audioScene);
	audioEngine->registerScene(audioScene);
	return audioScene.get();
}

SystemSceneInterface* AudioSystem::findSystemScene(const UScene* uscene)
{
	for (const auto& scene : audioScenes) {
		if (scene->uscene == uscene) return scene.get();
	}
	return nullptr;
}
