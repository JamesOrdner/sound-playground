#include "Engine/Engine.h"
#include "Engine/EModel.h"
#include "Audio/Components/AMicrophone.h"
#include "Audio/Components/ASpeaker.h"

int main(int argc, char* args[])
{
	Engine& engine = Engine::instance();
	EWorld& world = engine.world();

	for (int x = -2; x <= 2; x++) {
		for (int z = -1; z <= 2; z++) {
			auto model = std::make_shared<EModel>("res/platform.glb");
			model->setPosition(mat::vec3{ static_cast<float>(x), 0, static_cast<float>(z) - 0.5f });
			world.addObject(model);
		}
	}

	auto speaker = std::make_shared<EModel>("res/speaker_small.glb");
	speaker->setPosition(mat::vec3{ -2, 0, 0 });
	speaker->addAudioComponent(std::make_shared<ASpeaker>(), speaker);
	world.addObject(speaker);

	auto mic = std::make_shared<EObject>();
	mic->addAudioComponent(std::make_shared<AMicrophone>(), mic);
	world.addObject(mic);

	engine.run();
	return 0;
}
