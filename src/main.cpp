#include "Engine/Engine.h"

int main(int argc, char* args[])
{
	Engine engine;
	engine.init();
	engine.run();
	engine.deinit();

	//// run a second time to test
	//engine.init();
	//engine.run();
	//engine.deinit();

	return 0;
}
