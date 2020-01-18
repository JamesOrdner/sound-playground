#pragma once

class SystemInterface
{
public:

	SystemInterface();

	virtual ~SystemInterface();

	class AssetManagerInterface* assetManager;
	class ServiceManagerInterface* serviceManager;

	// Initialize the system
	virtual bool init() = 0;

	// Deinitialize the system
	virtual void deinit() = 0;

	// Execute a single run of the system, normally a single frame
	virtual void execute(float deltaTime) = 0;

	// Create a system scene and associate it with this system
	virtual class SystemSceneInterface* createSystemScene(const class UScene* uscene) = 0;

	// Return the system scene associated with this UScene
	virtual class SystemSceneInterface* findSystemScene(const class UScene* uscene) = 0;
};
