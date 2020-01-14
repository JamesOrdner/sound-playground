#pragma once

#include "../Managers/StateManager.h"
#include <list>
#include <memory>

// Forward declarations
class AudioComponent;
class OutputAudioComponent;
class AuralizingAudioComponent;
class EObject;

class AudioEngine
{
public:

	AudioEngine();

	~AudioEngine();

	// Create and return an audio component
	template<class T>
	T* createAudioComponent(const EObject* owner) {
		return static_cast<T*>(registerComponent(std::make_unique<T>(), owner));
	};

	// Set up AudioEngine and open the audio device. Returns success.
	bool init();

	// Close the audio device and deinitialize AudioEngine
	void deinit();

	// Begin processing and output. Returns success.
	bool start();

	// Stop processing and mute output
	void stop();

	// Internal callback called by the unscoped SDL callback
	void process_float(float* buffer, size_t frames);

private:

	// Pointer to the active stream (may be null)
	void* audioStream;
	
	// Current audio device sample rate
	float sampleRate;

	// Current number of channels
	int channels;


	std::list<std::unique_ptr<AudioComponent>> ownedComponents;

	// This list contains all audio components, sorted from least dependent to most dependent.
	// Dependency is determined from the input buffers. Components with longer input buffers
	// are considered less dependent, while components with shorter length input buffers
	// are more dependent. Components with no input buffers are the least dependent.
	std::list<AudioComponent*> components;

	// This list contains all OuputAudioComponents. Pointers to
	// these objects also exists in the `components` list.
	std::list<OutputAudioComponent*> outputComponents;

	// This list contains all AuralizingAudioComponents. Pointers to
	// these objects also exists in the `components` list.
	std::list<AuralizingAudioComponent*> auralizingComponents;

	// Registers an audio component with the engine for processing
	AudioComponent* registerComponent(std::unique_ptr<AudioComponent> component, const EObject* owner);

	// Called from the audio thread
	void registerComponent(const StateManager::EventData& data);
	void unregisterComponent(const StateManager::EventData& data);
};
