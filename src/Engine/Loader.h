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
		std::unique_ptr<class SystemInterface> audio;
	};

	SystemsWrapper createSystems();

	void loadDefaultScene(class UScene* uscene);

	class UObject* createObjectFromAsset(AssetID asset, class UScene* uscene) const override;
	class UObject* createObjectFromAsset(const AssetDescriptor& asset, class UScene* uscene) const override;
	class UObject* createUIObject(class UScene* uscene) const override;

	void deleteObject(const class UObject* uobject, class UScene* uscene) const override;

private:

	class SystemInterface* inputSystem;
	class SystemInterface* graphicsSystem;
	class SystemInterface* physicsSystem;
	class SystemInterface* audioSystem;

	class UObject* createDefaultCamera(class UScene* uscene);
};
