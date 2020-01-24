#pragma once

#include "../../Util/LFQueue.h"
#include <vector>
#include <memory>

class AudioEngine
{
public:

	AudioEngine();

	~AudioEngine();

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

	// This function is called at a regular interval outside of the audio thread
	void tick(float deltaTime);

	// Register a scene with the engine for processing
	void registerScene(std::shared_ptr<class AudioScene> scene);

	// Remove a scene from active processing
	void unregisterScene(class AudioScene* scene);

	// Registers and takes ownership of an audio component
	void registerComponent(std::unique_ptr<class AudioComponent> component, class AudioScene* scene);

	// Signal that a component is ready for removal from the audio graph and deletion
	void unregisterComponent(class AudioComponent* component, class AudioScene* scene);

private:

	// Pointer to the active stream (may be null)
	void* audioStream;

	// Current audio device sample rate
	float sampleRate;

	// Current number of channels
	int channels;

	struct ExternalAudioEngineEvent
	{
		enum class Type {
			SceneAdded,
			SceneRemoved,
			ComponentAdded,
			ComponentRemoved
		} type;
		class AudioComponent* component;
		class AudioScene* scene;
	};
	// This queue takes events from outside the audio thread and is parsed on the audio thread
	LFQueue<ExternalAudioEngineEvent> externalEventQueue;

	struct InternalAudioEngineEvent
	{
		enum class Type {
			DeleteScene,
			DeleteComponent
		} type;
		class AudioComponent* component;
		class AudioScene* scene;
	};
	// This queue takes events from the audio thread and is parsed outside the audio thread
	LFQueue<InternalAudioEngineEvent> internalEventQueue;

	// Contains all existing AudioComponents across all scenes, and is responsible for their deallocation
	std::vector<std::unique_ptr<class AudioComponent>> audioComponents;

	// All existing audio scenes
	std::vector<std::shared_ptr<class AudioScene>> scenes;

	// All scenes registered for active processing
	std::vector<class AudioScene*> activeScenes;
};
