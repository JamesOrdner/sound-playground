#include "EnvironmentManager.h"

EnvironmentManager::EnvironmentManager() :
	bQuitRequested(false)
{
}

EnvironmentManager& EnvironmentManager::instance()
{
	static EnvironmentManager instance;
	return instance;
}
