#pragma once

#include "../Managers/AssetTypes.h"

class LoaderInterface
{
public:

	virtual ~LoaderInterface() {};

	virtual class UObject* createObjectFromAsset(AssetID asset, class UScene* uscene) const = 0;
	virtual class UObject* createObjectFromAsset(const AssetDescriptor& asset, class UScene* uscene) const = 0;
	virtual class UObject* createUIObject(class UScene* uscene) const = 0;
};
