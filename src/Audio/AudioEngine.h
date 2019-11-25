#pragma once

#include <list>
#include <memory>

// Forward declarations
typedef unsigned __int32 SDL_AudioDeviceID;

class AudioComponent;

class AudioEngine
{
public:
	AudioEngine();

	// Set up AudioEngine and open the audio device. Returns success.
	bool init();

	// Close the audio device and deinitialize AudioEngine
	void deinit();

	// Begin processing and output. Returns success.
	bool start();

	// Stop processing and mute output
	void stop();

	// Internal callback called by the unscoped SDL callback
	void process_float(float* buffer, int length);

private:
	SDL_AudioDeviceID deviceID;
	
	// Current audio device sample rate
	int sampleRate;

	// Current number of channels
	int channels;

	// Length in frames of audio device buffer
	int bufferLength;

	// List contains all audio components, sorted from least dependent to most dependent.
	// Dependencies are determined by the input buffers. Components with no input buffers
	// are considered the least dependent, while shorter input buffers are more dependent.
	std::list<std::shared_ptr<AudioComponent>> components;
};
