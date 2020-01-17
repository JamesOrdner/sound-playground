#pragma once

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
