#pragma once

#include "../SystemInterface.h"
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

	SystemSceneInterface* createSystemScene(const class UScene* uscene) override;

	SystemSceneInterface* findSystemScene(const class UScene* uscene) override;

private:

	std::list<std::unique_ptr<class InputScene>> inputScenes;
};
