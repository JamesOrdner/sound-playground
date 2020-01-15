#include "Engine/Engine.h"

int main(int argc, char* args[])
{
	Engine& engine = Engine::instance();

	//for (int x = -2; x <= 2; x++) {
	//	for (int z = -1; z <= 2; z++) {
	//		EModel* platform = world.spawnObject<EModel>();
	//		platform->setMesh("res/platform.glb");
	//		platform->setPosition(mat::vec3{ static_cast<float>(x), 0, static_cast<float>(z) - 0.5f });
	//	}
	//}

	//EObject* mic = world.spawnObject<EObject>();
	//mic->audioComponent = engine.audio().createAudioComponent<AMicrophone>(mic);

	engine.run();
	return 0;
}
