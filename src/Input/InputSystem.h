#pragma once

#include "../Engine/SystemInterface.h"
#include <list>
#include <memory>

class InputSystem : public SystemInterface
{
public:

	InputSystem();

	~InputSystem();

	bool init() override;

	void deinit() override;

	void execute(float deltaTime) override;

private:

	std::list<std::unique_ptr<class InputScene>> inputScenes;

	SystemSceneInterface* addSystemScene(SystemSceneInterface* scene, const UScene* uscene) override;
};
