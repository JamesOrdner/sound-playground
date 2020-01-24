#include "OutputAudioComponent.h"
#include "AuralizingAudioComponent.h"

OutputAudioComponent::OutputAudioComponent()
{
}

OutputAudioComponent::~OutputAudioComponent()
{
}

void OutputAudioComponent::transformUpdated()
{
	AudioComponent::transformUpdated();
	for (auto send : indirectSends) send->auralize();
}
