#pragma once

#include "AssetTypes.h"
#include <vector>

class AssetManager
{
public:

	// Load all assets from the filesystem
	void loadAssets();

	// Assigns `id` based on the asset's name (assigns first if multiple found). Returns success.
	bool assetID(const std::string& name, AssetID& id) const;

	// Assigns `descriptor` from an asset name. Returns success.
	bool descriptor(const std::string& name, AssetDescriptor& descriptor) const;

	// Assigns `descriptor` from an AssetID. Returns success.
	bool descriptor(AssetID id, AssetDescriptor& descriptor) const;

private:

	std::vector<AssetDescriptor> assets;

	// Parse a line and fill in the appropriate descriptor field
	bool parseLine(const std::string& line, AssetDescriptor& descriptor);
};
