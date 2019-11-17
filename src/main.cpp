#include "Engine/Engine.h"

int main(int argc, char* args[])
{
	Engine& engine = Engine::instance();
	engine.run();
	return 0;
}
