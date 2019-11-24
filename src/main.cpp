#include "Engine/Engine.h"
#include "Engine/EModel.h"

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

	engine.run();
	return 0;
}
