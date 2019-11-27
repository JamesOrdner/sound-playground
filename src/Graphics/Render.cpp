#include "Render.h"
#include "GMesh.h"
#include <SDL.h>
#include <GL/gl3w.h>
#include <stdio.h>

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

	const char* vertex_shader =
		"#version 410\n"

		"layout(location = 0) in  vec3 position;"
		"layout(location = 1) in  vec3 normal;"
		"layout(location = 2) in  vec3 texCoord;"
		"layout(location = 3) in  mat4 instanceTransform;"

		"layout(location = 0) out vec3 outNormal;"
		"layout(location = 1) out vec3 outTexCoord;"

		"uniform mat4 viewProj;"

		"void main() {"
		"  gl_Position = viewProj * instanceTransform * vec4(position, 1.0);"
		"  outNormal = normal;"
		"  outTexCoord = texCoord;"
		"}";

	GLuint vs = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vs, 1, &vertex_shader, NULL);
	glCompileShader(vs);

	glGetShaderInfoLog(vs, 512, &errLen, err);
	if (*err) printf("%s\n", err);

	const char* fragment_shader =
		"#version 410\n"

		"layout(location = 0) in  vec3 normal;"
		"layout(location = 0) out vec4 color;"

		"void main() {"
		"  vec3 light = normalize(vec3(2, 3, -0.5));"
		"  float val = max(dot(light, normal), 0) + 0.1;"
		"  color = vec4(val, val, val, 1.0);"
		"}";

	GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fs, 1, &fragment_shader, NULL);
	glCompileShader(fs);

	glGetShaderInfoLog(fs, 512, &errLen, err);
	if (*err) printf("%s\n", err);

	glProgram = glCreateProgram();
	glAttachShader(glProgram, fs);
	glAttachShader(glProgram, vs);
	glLinkProgram(glProgram);
	glUseProgram(glProgram);

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
	glUniformMatrix4fv(viewProjMatrixID, 1, true, *projectionViewMatrix.data);

	return true;
}

void Render::deinit()
{
	glDeleteProgram(glProgram);
	glProgram = 0;
	SDL_GL_DeleteContext(glContext);
	glContext = nullptr;
}

void Render::draw(SDL_Window* window, const std::map<std::string, std::weak_ptr<GMesh>>& meshes)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	for (const auto& mesh : meshes) mesh.second.lock()->draw();
	SDL_GL_SwapWindow(window);
}

const mat::mat4& Render::screenToWorldMatrix()
{
	return invProjectionViewMatrix;
}
