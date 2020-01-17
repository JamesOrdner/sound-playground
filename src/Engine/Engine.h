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

	// Return a pointer to the Loader's public interface
	class LoaderInterface* loaderInterface() const;

private:

	// Register system interfaces with the managers
	void registerInterfaces();

	void setupInitialScene();

	// True only after a successful call to init()
	bool bInitialized;

	std::unique_ptr<class Loader> loader;

	std::unique_ptr<class SystemInterface> inputSystem;
	std::unique_ptr<class SystemInterface> graphicsSystem;
	std::unique_ptr<class SystemInterface> physicsSystem;

	std::list<std::unique_ptr<class UScene>> scenes;
};
