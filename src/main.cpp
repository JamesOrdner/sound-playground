#include "Engine/Engine.h"
#include "Engine/EModel.h"
#include "Audio/Components/AMicrophone.h"

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

	auto model = std::make_shared<EModel>("res/suzanne.glb");
	model->setPosition(mat::vec3{ 0, 1, 0 });
	model->addAudioComponent(std::make_shared<AMicrophone>());
	world.addObject(model);
	world.removeObject(model);

	engine.run();
	return 0;
}
