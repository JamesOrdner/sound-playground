#pragma once

// Forward declarations
typedef unsigned __int32 SDL_AudioDeviceID;

class AudioEngine
{
public:
	AudioEngine();

	// Set up AudioEngine and immediately start processing
	bool init();

	// Deinitialize AudioEngine
	void deinit();

	void process_float(float* buffer, int length);

private:
	SDL_AudioDeviceID deviceID;
	
	int sampleRate;
};
