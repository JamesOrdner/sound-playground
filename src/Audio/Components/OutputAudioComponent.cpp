#include "OutputAudioComponent.h"
#include "AuralizingAudioComponent.h"

OutputAudioComponent::OutputAudioComponent()
{
}

OutputAudioComponent::~OutputAudioComponent() = default;

void OutputAudioComponent::registerIndirectSend(IndirectSend* send)
{
	indirectSends.remove(send); // don't allow duplicates
	indirectSends.push_front(send);
}

void OutputAudioComponent::unregisterIndirectSend(IndirectSend* send)
{
	indirectSends.remove(send);
}

void OutputAudioComponent::transformUpdated()
{
	AudioComponent::transformUpdated();
	for (auto send : indirectSends) send->auralize();
}
