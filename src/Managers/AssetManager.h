#pragma once

#include <vector>
#include <string>

enum class AssetType
{
	Object
};

enum class AudioType
{
	None,
	Speaker,
	Microphone
};

typedef size_t AssetID;

struct AssetDescriptor
{
	std::string name;
	AssetType assetType;
	AudioType audioType;
	std::string modelPath;
	std::string uiImagePath;
	AssetID assetID;
};

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
