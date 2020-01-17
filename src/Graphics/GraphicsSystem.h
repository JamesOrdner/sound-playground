#pragma once

#include "../Engine/SystemInterface.h"
#include <list>
#include <memory>

class GraphicsSystem : public SystemInterface
{
public:

	GraphicsSystem();

	~GraphicsSystem();

	bool init() override;

	void deinit() override;

	void execute(float deltaTime) override;

	SystemSceneInterface* createSystemScene(const class UScene* uscene) override;

	SystemSceneInterface* findSystemScene(const class UScene* uscene) override;

private:

	struct SDL_Window* window;

	std::unique_ptr<class Render> render;

	std::list<std::unique_ptr<class GraphicsScene>> graphicsScenes;
};
