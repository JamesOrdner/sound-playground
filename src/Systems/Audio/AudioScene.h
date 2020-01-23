#pragma once

#include "../SystemSceneInterface.h"
#include <list>
#include <memory>

class AudioScene : public SystemSceneInterface
{
public:

	AudioScene(const class SystemInterface* system, class AudioEngine* audioEngine, const class UScene* uscene);

	~AudioScene();

	void deleteSystemObject(const class UObject* uobject) override;

	template <typename T>
	T* setAudioComponentForObject(class AudioObject* object)
	{
		return static_cast<T*>(addAudioComponentToObject(std::make_unique<T>(), object));
	}

	// Called from the audio thread. Process and constructively add count `frames` to the provided interleaved buffer.
	void processSceneAudio(float* buffer, size_t frames);

	// Called from the audio thread. Connect a component to the audio graph for processing.
	void connectAudioComponent(class AudioComponent* component);

	// Called from the audio thread. Disconnect a component from the audio graph.
	void disconnectAudioComponent(class AudioComponent* component);

	// Returns the number of audio components in use by the scene
	size_t registeredComponentCount() const;

private:

	SystemObjectInterface* addSystemObject(SystemObjectInterface* object) override;

	class AudioComponent* addAudioComponentToObject(std::unique_ptr<class AudioComponent> component, class AudioObject* object);

	class AudioEngine* const audioEngine;

	std::list<std::unique_ptr<class AudioObject>> audioObjects;

	// This list contains all active audio components in this scene
	std::list<class AudioComponent*> components;

	// This list contains all OuputAudioComponents. Pointers to
	// these objects also exists in the `components` list.
	std::list<class OutputAudioComponent*> outputComponents;

	// This list contains all AuralizingAudioComponents. Pointers to
	// these objects also exists in the `components` list.
	std::list<class AuralizingAudioComponent*> auralizingComponents;
};
