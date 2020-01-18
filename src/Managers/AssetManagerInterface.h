#pragma once

#include "AssetTypes.h"

class AssetManagerInterface
{
public:

	// Assigns `id` based on the asset's name (assigns first if multiple found). Returns success.
	virtual bool assetID(const std::string& name, AssetID& id) const = 0;

	// Assigns `descriptor` from an asset name. Returns success.
	virtual bool descriptor(const std::string& name, AssetDescriptor& descriptor) const = 0;

	// Assigns `descriptor` from an AssetID. Returns success.
	virtual bool descriptor(AssetID id, AssetDescriptor& descriptor) const = 0;
};