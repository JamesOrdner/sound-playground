#pragma once

#include <list>
#include <memory>

class Engine
{
public:

	Engine();

	~Engine();

	bool init();

	void deinit();

	// Main runloop, returns on user exit
	void run();

private:

	void setupInitialScene();

	// True only after a successful call to init()
	bool bInitialized;

	std::list<std::unique_ptr<class UScene>> scenes;

	std::unique_ptr<class SystemInterface> inputSystem;
	std::unique_ptr<class SystemInterface> graphicsSystem;
};
