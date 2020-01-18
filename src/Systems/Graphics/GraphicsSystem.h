#pragma once

#include "../SystemInterface.h"
#include "GraphicsSystemInterface.h"
#include <list>
#include <memory>

class GraphicsSystem : public SystemInterface, public GraphicsSystemInterface
{
public:

	GraphicsSystem();

	~GraphicsSystem();

	// SystemInterface

	bool init() override;
	void deinit() override;
	void execute(float deltaTime) override;
	SystemSceneInterface* createSystemScene(const class UScene* uscene) override;
	SystemSceneInterface* findSystemScene(const class UScene* uscene) override;

	// GraphicsSystemInterface

	void screenDimensions(int& x, int& y) const override;
	const mat::mat4& screenToWorldTransform(const class UScene* uscene) const override;

private:

	struct SDL_Window* window;

	std::unique_ptr<class Render> render;

	std::list<std::unique_ptr<class GraphicsScene>> graphicsScenes;
};
