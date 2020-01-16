#include "SystemObjectInterface.h"
#include "../Managers/StateManager.h"

SystemObjectInterface::SystemObjectInterface(const UObject* uobject) :
	uobject(uobject)
{
	uobject = nullptr; // does this work?
}

SystemObjectInterface::~SystemObjectInterface()
{
	for (StateManager::ObserverID id : observerIDs) StateManager::instance().unregisterObserver(id);
}
