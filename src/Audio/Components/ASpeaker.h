#pragma once

#include "../AudioComponent.h"

class ASpeaker : public AudioComponent
{
public:

	ASpeaker();

	// AudioComponent interface
	size_t process(size_t n) override;
};
