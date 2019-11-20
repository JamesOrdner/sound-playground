#include "Engine/Engine.h"
#include "Engine/EModel.h"

int main(int argc, char* args[])
{
	Engine& engine = Engine::instance();

	auto model1 = std::make_shared<EModel>("res/platform.glb");
	model1->setLocation(mat::vec3{ 0, 0, 1 });
	engine.registerModel(model1);

	auto model2 = std::make_shared<EModel>("res/platform.glb");
	model2->setLocation(mat::vec3{ 0, 0, 0 });
	engine.registerModel(model2);

	engine.run();
	return 0;
}
