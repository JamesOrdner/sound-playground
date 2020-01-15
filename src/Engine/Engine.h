#pragma once

#include <list>
#include <memory>

class Engine
{
public:

	// Main runloop, returns on user exit
	void run();

private:

	Engine();
	
	~Engine();

	bool init();

	void deinit();

	void setupInitialScene();

	// True only after a successful call to init()
	bool bInitialized;

	// All scenes
	std::list<std::unique_ptr<class UScene>> scenes;

	std::unique_ptr<class SystemInterface> inputSystem;
	std::unique_ptr<class SystemInterface> graphicsSystem;

public:

	static Engine& instance();

	// Deleted functions prevent singleton duplication
	Engine(Engine const&) = delete;
	void operator=(Engine const&) = delete;
};
