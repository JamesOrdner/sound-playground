#pragma once

#include "../AudioOutputComponent.h"

class AMicrophone : public AudioOutputComponent
{
public:

	AMicrophone();

	// Points to the current location in the outputBuffer, reset each preprocess()
	size_t outputPtr;

	// AudioComponent interface
	void preprocess() override;
	size_t process(size_t n) override;
};
