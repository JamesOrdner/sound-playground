#pragma once

#include <list>

class Auralizer
{
public:

	void auralize();

private:

	std::list<class AuralizingAudioComponent*> auralizingComponents;
};
