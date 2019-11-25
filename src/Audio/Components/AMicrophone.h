#pragma once

#include "../AudioOutputComponent.h"

class AMicrophone : public AudioOutputComponent
{
public:

	// AudioComponent interface
	void process(size_t n) override;
};
