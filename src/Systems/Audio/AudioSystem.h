#pragma once

#include "../SystemInterface.h"
#include <vector>
#include <memory>

class AudioSystem : public SystemInterface
{
public:

	AudioSystem();

	~AudioSystem();

	// SystemInterface
	bool init() override;
	void deinit() override;
	void execute(float deltaTime) override;
	SystemSceneInterface* createSystemScene(const class UScene* uscene) override;
	SystemSceneInterface* findSystemScene(const class UScene* uscene) override;

private:

	// AudioScene objects are shared by AudioEngine
	std::vector<std::shared_ptr<class AudioScene>> audioScenes;

	std::unique_ptr<class AudioEngine> audioEngine;
};
