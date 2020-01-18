#pragma once

#include "AssetManagerInterface.h"
#include <vector>

class AssetManager : public AssetManagerInterface
{
public:

	// AssetManagerInterface

	bool assetID(const std::string& name, AssetID& id) const override;
	bool descriptor(const std::string& name, AssetDescriptor& descriptor) const override;
	bool descriptor(AssetID id, AssetDescriptor& descriptor) const override;

private:

	AssetManager();

	std::vector<AssetDescriptor> assets;

	// Load all assets from the filesystem
	void loadAssets();

	// Parse a line and fill in the appropriate descriptor field
	bool parseLine(const std::string& line, AssetDescriptor& descriptor);

public:

	static AssetManager& instance();

	// Deleted functions prevent singleton duplication
	AssetManager(AssetManager const&) = delete;
	void operator=(AssetManager const&) = delete;
};
