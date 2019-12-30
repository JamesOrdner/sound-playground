#pragma once

#include "Matrix.h"
#include <string>
#include <functional>

class GProgram
{
public:

	GProgram();

	// Construct the shader with the name of the glsl shader file (.vert/.frag)
	GProgram(std::string name);

	~GProgram();

	// Set this program as the active program
	void use();

	// Perform any post-draw cleanup, as well as unbinding the program if desired. If no release
	// routine has been set and unbinding is not desired, calling this function is not required
	void release(bool unbind = false);

	// Provide a function that will be called when use() is called
	void setPreDrawRoutine(const std::function<void()>& f);

	// Provide a function that will be called when release() is called
	void setReleaseRoutine(const std::function<void()>& f);

	// Set a matrix uniform with the given name to the given value
	void setMatrixUniform(std::string name, const mat::mat4& matrix);

	// Store the framebuffer that this program renders to, and bind the framebuffer when in use
	void setFramebuffer(unsigned int framebuffer);

private:

	// Read the contents of a text file at the specified path into a string
	std::string loadTextFile(std::string filepath);

	// Shader program
	unsigned int program;

	// Framebuffer address, valid only if bUseFramebuffer == true
	unsigned int fbo;

	// Indicates whether this program renders to a framebuffer
	bool bUseFramebuffer;

	// Routine called when program is bound
	std::function<void()> predrawRoutine;

	// Routine called when program is released
	std::function<void()> releaseRoutine;
};

