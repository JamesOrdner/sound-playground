#pragma once

class EnvironmentManager
{
public:

	bool bQuitRequested;

private:

	EnvironmentManager();

public:

	static EnvironmentManager& instance();

	// Deleted functions prevent singleton duplication
	EnvironmentManager(EnvironmentManager const&) = delete;
	void operator=(EnvironmentManager const&) = delete;
};

