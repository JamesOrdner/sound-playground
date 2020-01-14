#include "ESpeaker.h"
#include "../Engine/Engine.h"
#include "../Audio/AudioEngine.h"
#include "../Audio/Components/ASpeaker.h"
#include "../UI/UIComponent.h"

ESpeaker::ESpeaker()
{
	setMesh("res/speaker_small.glb");

	audioComponent = Engine::instance().audio().createAudioComponent<ASpeaker>(this);

	m_uiComponent = std::make_unique<UIComponent>();
	UIData& data = m_uiComponent->data.emplace_back();
	data.type = UIType::Button;
	data.value.boolVal = false;
}
