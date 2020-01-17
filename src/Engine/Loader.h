#pragma once

#include "LoaderInterface.h"
#include <memory>

class Loader : public LoaderInterface
{
public:

	Loader();

	struct SystemsWrapper
	{
		std::unique_ptr<class SystemInterface> input;
		std::unique_ptr<class SystemInterface> graphics;
		std::unique_ptr<class SystemInterface> physics;
	};

	SystemsWrapper createSystems();

	class UObject* createDefaultCamera(class UScene* uscene);

	class UObject* createObjectFromAsset(AssetID asset, class UScene* uscene) const override;
	class UObject* createObjectFromAsset(const AssetDescriptor& asset, class UScene* uscene) const override;

private:

	class AssetManager* assetManager;

	class SystemInterface* inputSystem;
	class SystemInterface* graphicsSystem;
};
