#include "AWAVFile.h"
#include <fstream>

AWAVFile::AWAVFile(std::string filepath) :
	channels(0),
	sampleRate(0)
{
	std::ifstream fs(filepath, std::ios_base::in | std::ios_base::binary);
	if (!fs) {
		printf("Unable to open file: %s\n", filepath.c_str());
		return;
	}

	char header[44];
	fs.read(header, sizeof(header));
	if (!fs) {
		printf("Invalid wav file: %s\n", filepath.c_str());
		return;
	}

	if (strncmp(header + 0, "RIFF", 4) ||
		strncmp(header + 8, "WAVE", 4) ||
		strncmp(header + 12, "fmt ", 4) ||
		strncmp(header + 36, "data", 4) ||
		*(uint32_t*)(header + 16) != 16 || // PCM chunk size
		*(uint16_t*)(header + 20) != 1 ||  // PCM format
		*(uint16_t*)(header + 34) != 16)   // bit depth
	{
		printf("Invalid wav header: %s\n", filepath.c_str());
		return;
	}

	channels = *(uint16_t*)(header + 22);
	sampleRate = *(uint32_t*)(header + 24);

	uint32_t byteCount = *(uint32_t*)(header + 40);
	uint32_t sampleCount = byteCount / 2;
	data.reserve(sampleCount);

	int16_t* buffer = new int16_t[sampleCount];
	fs.read((char*)buffer, byteCount);
	if (!fs) {
		printf("Error reading wav data: %s\n", filepath.c_str());
		data.resize(0);
		return;
	}

	for (std::streamsize i = 0; i < sampleCount; i++) {
		float raw = static_cast<float>(buffer[i]);
		data.push_back(raw / INT16_MAX);
	}

	delete[] buffer;
}
