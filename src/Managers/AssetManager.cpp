#include "AssetManager.h"
#include <fstream>
#include <iostream>
#include <filesystem>
#include <regex>

AssetManager& AssetManager::instance()
{
	static AssetManager instance;
	return instance;
}

AssetManager::AssetManager()
{
	loadAssets();
}

void AssetManager::loadAssets()
{
	namespace fs = std::filesystem;
	for (auto& file : fs::directory_iterator("res/assets")) {
		std::ifstream fs(file, std::ios_base::in);
		AssetDescriptor asset = {};
		std::string line;
		bool bSuccess = true;
		while (std::getline(fs, line)) {
			if (!parseLine(line, asset)) {
				std::cout << "Warning: Invalid asset file " << file.path() << std::endl;
				bSuccess = false;
				break;
			}
		}
		if (bSuccess) {
			asset.assetID = assets.size(); // assumes no asset removal
			assets.push_back(asset);
		}
	}
}

inline bool setAssetName(const std::string& s, std::string& name)
{
	name = s;
	return true;
}

inline bool setAssetType(const std::string& s, AssetType& type)
{
	if (s == "Object") type = AssetType::Object;
	else return false;
	return true;
}

inline bool setAudioType(const std::string& s, AudioType& type)
{
	if (s == "Speaker") type = AudioType::Speaker;
	else if (s == "Microphone") type = AudioType::Microphone;
	else return false;
	return true;
}

inline bool setAssetModelPath(const std::string& s, std::string& path)
{
	path = "res/models/" + s;
	return std::filesystem::exists(path);
}

bool AssetManager::parseLine(const std::string& line, AssetDescriptor& descriptor)
{
	std::smatch match;
	if (std::regex_search(line, match, std::regex("="))) {
		const std::string& key = match.prefix();
		const std::string& val = match.suffix();
		if (key == "Name")
			return setAssetName(val, descriptor.name);
		if (key == "AssetType")
			return setAssetType(val, descriptor.assetType);
		if (key == "AudioType")
			return setAudioType(val, descriptor.audioType);
		if (key == "Model")
			return setAssetModelPath(val, descriptor.modelPath);
	}

	return false;
}

bool AssetManager::assetID(const std::string& name, AssetID& id) const
{
	for (id = 0; id < assets.size(); id++) {
		if (assets[id].name == name) return true;
	}
	return false;
}

bool AssetManager::descriptor(const std::string& name, AssetDescriptor& descriptor) const
{
	for (size_t i = 0; i < assets.size(); i++) {
		if (assets[i].name == name) {
			descriptor = assets[i];
			return true;
		}
	}
	return false;
}

bool AssetManager::descriptor(AssetID id, AssetDescriptor& descriptor) const
{
	if (id < assets.size()) {
		descriptor = assets[id];
		return true;
	}
	return false;
}
