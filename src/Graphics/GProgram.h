#pragma once

#include "../Util/Matrix.h"
#include <string>
#include <functional>
#include <map>

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

	void setUniform(std::string name, const mat::vec4& vector);
	void setUniform(std::string name, const mat::mat3& matrix);
	void setUniform(std::string name, const mat::mat4& matrix);
	void setUniform(std::string name, const std::vector<mat::vec3>& vectors);

	// Return the GLuint location of a glsl uniform
	unsigned int getUniformLocation(std::string name);

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

	// This map is built dynamically each time glGetUniformLocation is called
	std::map<std::string, unsigned int> uniformLocations;
};
