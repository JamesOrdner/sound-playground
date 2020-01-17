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
	const class LoaderInterface* loader() const;

private:

	void setupInitialScene();

	// True only after a successful call to init()
	bool bInitialized;

	std::list<std::unique_ptr<class UScene>> scenes;

	std::unique_ptr<class SystemInterface> inputSystem;
	std::unique_ptr<class SystemInterface> graphicsSystem;

	std::unique_ptr<class AssetManager> assetManager;
	std::unique_ptr<class Loader> assetLoader;
};
