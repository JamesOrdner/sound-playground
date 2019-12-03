#pragma once

#include <list>
#include <memory>

// Forward declarations
typedef unsigned __int32 SDL_AudioDeviceID;
class AudioComponent;
class EObject;

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

	// Registers an audio component with the engine for processing
	void registerComponent(
		const std::shared_ptr<AudioComponent>& component,
		const std::shared_ptr<EObject>& owner);

	// Removes an audio component from the engine
	void unregisterComponent(const std::shared_ptr<AudioComponent>& component);

private:

	// An integer representing the current audio device
	SDL_AudioDeviceID deviceID;
	
	// Current audio device sample rate
	float sampleRate;

	// Current number of channels
	int channels;

	// Length in frames of audio device buffer
	int bufferLength;

	// This list contains all audio components, sorted from least dependent to most dependent.
	// Dependency is determined from the input buffers. Components with longer input buffers
	// are considered less dependent, while components with shorter length input buffers
	// are more dependent. Components with no input buffers are the least dependent.
	std::list<std::shared_ptr<AudioComponent>> components;
};
