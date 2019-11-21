#include "Engine/Engine.h"
#include "Engine/EModel.h"

int main(int argc, char* args[])
{
	Engine& engine = Engine::instance();
	EWorld& world = engine.world();

	auto model1 = std::make_shared<EModel>("res/platform.glb");
	model1->setPosition(mat::vec3{ 0, 0, 1 });
	world.addObject(model1);

	auto model2 = std::make_shared<EModel>("res/platform.glb");
	model2->setPosition(mat::vec3{ 0, 0, 0 });
	world.addObject(model2);

	auto model3 = std::make_shared<EModel>("res/platform.glb");
	model3->setPosition(mat::vec3{ 1, 0, 0 });
	world.addObject(model3);

	engine.run();
	return 0;
}
