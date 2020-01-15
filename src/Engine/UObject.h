#pragma once

#include <vector>

// Forward declarations
class SystemObjectInterface;

class UObject
{
private:

	std::vector<SystemObjectInterface*> systemObjects;
};
