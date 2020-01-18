#include "EnvironmentManager.h"

EnvironmentManager::EnvironmentManager()
{
}

EnvironmentManager& EnvironmentManager::instance()
{
	static EnvironmentManager instance;
	return instance;
}
