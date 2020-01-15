#include "EComponent.h"

EComponent::EComponent()
{
}

EComponent::~EComponent()
{
	for (auto id : observerIDs) {
		StateManager::instance().unregisterObserver(id);
	}
}
