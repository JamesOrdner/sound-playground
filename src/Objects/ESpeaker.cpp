#include "ESpeaker.h"
#include "../Audio/Components/ASpeaker.h"
#include "../UI/UIComponent.h"

ESpeaker::ESpeaker()
{
	setMesh("res/speaker_small.glb");
	addAudioComponent(std::make_unique<ASpeaker>());

	m_uiComponent = std::make_unique<UIComponent>();
	UIData& data = m_uiComponent->data.emplace_back();
	data.type = UIType::Button;
	data.value.boolVal = false;
}
