#include "Render.h"
#include "GMesh.h"
#include <SDL.h>
#include <GL/gl3w.h>
#include <stdio.h>
#include<fstream>

constexpr int glMajorVersion = 4;
constexpr int glMinorVersion = 6;

Render::Render() :
	glContext(nullptr),
	glProgram(0)
{
}

void Render::setAttributes()
{
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 6);
	SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
	SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 4);
}

bool Render::init(SDL_Window* window)
{
	glContext = SDL_GL_CreateContext(window);

	// Init gl3w
	if (gl3wInit()) {
		fprintf(stderr, "failed to initialize OpenGL\n");
		return false;
	}
	if (!gl3wIsSupported(glMajorVersion, glMinorVersion)) {
		fprintf(stderr, "OpenGL %d.%d not supported\n", glMajorVersion, glMinorVersion);
		return false;
	}

	// Use Vsync
	if (SDL_GL_SetSwapInterval(1) < 0) {
		printf("Warning: Unable to set VSync! SDL Error: %s\n", SDL_GetError());
		return false;
	}

	glEnable(GL_MULTISAMPLE);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	char err[512];
	GLsizei errLen;

	GLuint vs = glCreateShader(GL_VERTEX_SHADER);
	std::string vertShader = loadTextFile("res/shaders/main.vert");
	const char* vertShaderC = vertShader.c_str();
	glShaderSource(vs, 1, &vertShaderC, 0);
	glCompileShader(vs);
	glGetShaderInfoLog(vs, 512, &errLen, err);
	if (*err) printf("%s\n", err);

	GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
	std::string fragShader = loadTextFile("res/shaders/main.frag");
	const char* fragShaderC = fragShader.c_str();
	glShaderSource(fs, 1, &fragShaderC, 0);
	glCompileShader(fs);
	glGetShaderInfoLog(fs, 512, &errLen, err);
	if (*err) printf("%s\n", err);

	glProgram = glCreateProgram();
	glAttachShader(glProgram, fs);
	glAttachShader(glProgram, vs);
	glLinkProgram(glProgram);
	glDeleteShader(fs);
	glDeleteShader(vs);

	int width, height;
	SDL_GL_GetDrawableSize(window, &width, &height);
	float aspectRatio = static_cast<float>(height) / static_cast<float>(width);
	GLuint viewProjMatrixID = glGetUniformLocation(glProgram, "viewProj");
	mat::mat4 view = lookAt(mat::vec3{ 0.f, 10.f, 4.f }, mat::vec3{ 0.f, 0.f, 0.f });
	mat::mat4 proj = mat::ortho(-3.5f, 3.5f, -3.5f * aspectRatio, 3.5f * aspectRatio, -50.f, 0.f);
	mat::mat4 projectionViewMatrix = proj * view;
	invProjectionViewMatrix = mat::inverse(projectionViewMatrix);
	glProgramUniformMatrix4fv(glProgram, viewProjMatrixID, 1, true, *projectionViewMatrix.data);

	if (!initBuffers()) return false;

	return true;
}

void Render::deinit()
{
	glDeleteProgram(glProgram);
	glDeleteProgram(glShadowProgram);
	glDeleteTextures(1, &shadowTexture);
	glProgram = 0;
	glShadowProgram = 0;
	shadowTexture = 0;

	SDL_GL_DeleteContext(glContext);
	glContext = nullptr;
}

void Render::draw(SDL_Window* window, const std::map<std::string, std::weak_ptr<GMesh>>& meshes)
{
	glEnable(GL_POLYGON_OFFSET_FILL);
	glUseProgram(glShadowProgram);
	glBindFramebuffer(GL_FRAMEBUFFER, shadowBuffer);
	glClear(GL_DEPTH_BUFFER_BIT);
	glViewport(0, 0, 1024, 1024);
	for (const auto& mesh : meshes) mesh.second.lock()->draw();
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
 	glDisable(GL_POLYGON_OFFSET_FILL);

	glUseProgram(glProgram);
	glViewport(0, 0, 1280, 720);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	for (const auto& mesh : meshes) mesh.second.lock()->draw();
	SDL_GL_SwapWindow(window);
}

const mat::mat4& Render::screenToWorldMatrix()
{
	return invProjectionViewMatrix;
}

bool Render::initBuffers()
{
	glGenFramebuffers(1, &shadowBuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, shadowBuffer);

	glGenTextures(1, &shadowTexture);
	glBindTexture(GL_TEXTURE_2D, shadowTexture);
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_DEPTH_COMPONENT32, 1024, 1024);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, shadowTexture, 0);
	glBindTextureUnit(0, shadowTexture);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) return false;

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	char err[512];
	GLsizei errLen;

	GLuint vs = glCreateShader(GL_VERTEX_SHADER);
	std::string vertShader = loadTextFile("res/shaders/shadow.vert");
	const char* vertShaderC = vertShader.c_str();
	glShaderSource(vs, 1, &vertShaderC, 0);
	glCompileShader(vs);
	glGetShaderInfoLog(vs, 512, &errLen, err);
	if (*err) printf("%s\n", err);

	GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
	std::string fragShader = loadTextFile("res/shaders/shadow.frag");
	const char* fragShaderC = fragShader.c_str();
	glShaderSource(fs, 1, &fragShaderC, 0);
	glCompileShader(fs);
	glGetShaderInfoLog(fs, 512, &errLen, err);
	if (*err) printf("%s\n", err);

	glShadowProgram = glCreateProgram();
	glAttachShader(glShadowProgram, fs);
	glAttachShader(glShadowProgram, vs);
	glLinkProgram(glShadowProgram);
	glDeleteShader(fs);
	glDeleteShader(vs);

	GLuint viewProjMatrixID = glGetUniformLocation(glShadowProgram, "mvp");
	mat::mat4 view = lookAt(mat::vec3{ 2.f, 3.f, -0.5f }, mat::vec3{ 0.f, 0.f, 0.f });
	mat::mat4 proj = mat::ortho(-3.f, 3.f, -3.f, 3.f, -10.f, 10.f);
	mat::mat4 projectionViewMatrix = proj * view;
	glProgramUniformMatrix4fv(glShadowProgram, viewProjMatrixID, 1, true, *projectionViewMatrix.data);

	GLuint shadowMatrixID = glGetUniformLocation(glProgram, "shadowMVP");
	mat::mat4 biasMatrix{
		{ 0.5, 0.0, 0.0, 0.5 },
		{ 0.0, 0.5, 0.0, 0.5 },
		{ 0.0, 0.0, 0.5, 0.5 },
		{ 0.0, 0.0, 0.0, 1.0 }
	};
	mat::mat4 depthBiasMVP = biasMatrix * projectionViewMatrix;
	glProgramUniformMatrix4fv(glProgram, shadowMatrixID, 1, true, *depthBiasMVP.data);

	glPolygonOffset(2.f, 2.f);
}

std::string Render::loadTextFile(std::string filepath)
{
	std::string out, line;
	std::ifstream file(filepath);
	if (!file.is_open()) return out;
	while (getline(file, line)) out.append(line + '\n');
	file.close();
	return out;
}
