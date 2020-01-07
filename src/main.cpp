#include "Engine/Engine.h"
#include "Engine/EWorld.h"
#include "Engine/EModel.h"
#include "Audio/Components/AMicrophone.h"
#include "Audio/Components/ASpeaker.h"

int main(int argc, char* args[])
{
	Engine& engine = Engine::instance();
	EWorld& world = engine.world();

	for (int x = -2; x <= 2; x++) {
		for (int z = -1; z <= 2; z++) {
			EModel* platform = world.spawnObject<EModel>();
			platform->setMesh("res/platform.glb");
			platform->setPosition(mat::vec3{ static_cast<float>(x), 0, static_cast<float>(z) - 0.5f });
		}
	}

	EObject* mic = world.spawnObject<EObject>();
	mic->addAudioComponent(std::make_unique<AMicrophone>());

	EModel* speaker = world.spawnObject<EModel>();
	speaker->setMesh("res/speaker_small.glb");
	speaker->setPosition(mat::vec3{ -2, 0, 0 });
	speaker->setRotation(mat::vec3{ 0, mat::pi * 0.5f, 0});
	speaker->addAudioComponent(std::make_unique<ASpeaker>());

	//auto speaker2 = std::make_shared<EModel>("res/speaker_small.glb");
	//speaker2->setPosition(mat::vec3{ 2, 0, 0 });
	//speaker2->setRotation(mat::vec3{ 0, mat::pi * 0.25f, 0 });
	//speaker2->addAudioComponent(std::make_shared<ASpeaker>(), speaker2);
	//world.addObject(speaker2);

	engine.run();
	return 0;
}
