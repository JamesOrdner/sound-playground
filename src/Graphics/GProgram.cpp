#include "GProgram.h"
#include <GL/gl3w.h>
#include <stdio.h>
#include <fstream>

GProgram::GProgram() :
	program(0),
	fbo(0),
	bUseFramebuffer(false)
{
}

GProgram::GProgram(std::string name) : GProgram()
{
	char err[512];
	GLsizei errLen;

	GLuint vs = glCreateShader(GL_VERTEX_SHADER);
	std::string vertShader = loadTextFile("res/shaders/" + name + ".vert");
	const char* vertShaderC = vertShader.c_str();
	glShaderSource(vs, 1, &vertShaderC, 0);
	glCompileShader(vs);
	glGetShaderInfoLog(vs, 512, &errLen, err);
	if (*err) printf("%s\n", err);

	GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
	std::string fragShader = loadTextFile("res/shaders/" + name + ".frag");
	const char* fragShaderC = fragShader.c_str();
	glShaderSource(fs, 1, &fragShaderC, 0);
	glCompileShader(fs);
	glGetShaderInfoLog(fs, 512, &errLen, err);
	if (*err) printf("%s\n", err);

	program = glCreateProgram();
	glAttachShader(program, fs);
	glAttachShader(program, vs);
	glLinkProgram(program);
	glDeleteShader(fs);
	glDeleteShader(vs);
}

GProgram::~GProgram()
{
	glDeleteProgram(program);
}

void GProgram::use()
{
	glUseProgram(program);
	if (bUseFramebuffer) glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	if (predrawRoutine) predrawRoutine();
}

void GProgram::release(bool unbind)
{
	if (releaseRoutine) releaseRoutine();
	if (bUseFramebuffer) glBindFramebuffer(GL_FRAMEBUFFER, 0);
	if (unbind) glUseProgram(0);
}

void GProgram::setPreDrawRoutine(const std::function<void()>& f)
{
	predrawRoutine = f;
}

void GProgram::setReleaseRoutine(const std::function<void()>& f)
{
	releaseRoutine = f;
}

void GProgram::setUniform(std::string name, const mat::vec4& vector)
{
	GLuint id = getUniformLocation(name);
	glProgramUniform4fv(program, id, 1, vector.data);
}

void GProgram::setUniform(std::string name, const mat::mat3& matrix)
{
	GLuint id = getUniformLocation(name);
	glProgramUniformMatrix3fv(program, id, 1, true, *matrix.data);
}

void GProgram::setUniform(std::string name, const mat::mat4& matrix)
{
	GLuint id = getUniformLocation(name);
	glProgramUniformMatrix4fv(program, id, 1, true, *matrix.data);
}

void GProgram::setUniform(std::string name, const std::vector<mat::vec3>& vectors)
{
	GLuint id = getUniformLocation(name);
	glProgramUniform3fv(program, id, vectors.size(), vectors[0].data);
}

void GProgram::setFramebuffer(unsigned int framebuffer)
{
	fbo = framebuffer;
	bUseFramebuffer = true;
}

std::string GProgram::loadTextFile(std::string filepath)
{
	std::string out, line;
	std::ifstream file(filepath);
	if (!file.is_open()) return out;
	while (getline(file, line)) out.append(line + '\n');
	file.close();
	return out;
}

unsigned int GProgram::getUniformLocation(std::string name)
{
	auto it = uniformLocations.find(name);
	if (it != uniformLocations.end()) return it->second;
	unsigned int location = glGetUniformLocation(program, name.c_str());
	uniformLocations[name] = location;
	return location;
}
