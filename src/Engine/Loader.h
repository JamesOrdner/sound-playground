#pragma once

#include "LoaderInterface.h"

class Loader : public LoaderInterface
{
public:

	Loader(
		const class AssetManager* assetManager,
		class SystemInterface* inputSystem,
		class SystemInterface* graphicsSystem);

	class UObject* createDefaultCamera(class UScene* uscene);

	class UObject* createObjectFromAsset(AssetID asset, class UScene* uscene) const override;
	class UObject* createObjectFromAsset(const AssetDescriptor& asset, class UScene* uscene) const override;

private:

	const class AssetManager* const assetManager;

	class SystemInterface* const inputSystem;
	class SystemInterface* const graphicsSystem;
};
