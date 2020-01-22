#pragma once

#include <string>
#include <vector>

struct AWAVFile
{
	AWAVFile(std::string filepath);
	uint16_t channels;
	uint32_t sampleRate;
	std::vector<float> data;
};
