#include "Engine.h"
#include "../Graphics/Matrix.h"
#include "../Graphics/GMesh.h"
#include <SDL.h>
#include <GL/gl3w.h>
#include <stdio.h>

Engine& Engine::instance()
{
	static Engine instance;
	return instance;
}

void Engine::run()
{
	if (!bInitialized) return;
	bool quit = false;
	while (!quit) {
		SDL_Event event;
		while (SDL_PollEvent(&event)) {
			switch (event.type) {
			case SDL_QUIT:
				quit = true;
				break;
			}
		}

		render();
	}
}

Engine::Engine()
{
	bInitialized = init();
}

Engine::~Engine()
{
	deinit();
}

bool Engine::init()
{
	int glMajorVersion = 4;
	int glMinorVersion = 6;

	// Initialize SDL
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
		printf("SDL could not initialize! SDL Error: %s\n", SDL_GetError());
		return false;
	}

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, glMajorVersion);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, glMinorVersion);

	// Create window
	sdlWindow = SDL_CreateWindow("Sound Playground", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 512, 512, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);
	if (!sdlWindow) {
		printf("Window could not be created! SDL Error: %s\n", SDL_GetError());
		return false;
	}

	glContext = SDL_GL_CreateContext(sdlWindow);

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

	if (!initGL()) {
		printf("Warning: initGL() returned false!\n");
		return false;
	}
	
	// Init audio system
	return audioEngine.init();
}

bool Engine::initGL()
{
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	char err[512];
	GLsizei errLen;

	const char* vertex_shader =
		"#version 410\n"

		"layout(location = 0) in  vec3 vertexPosition;"
		"layout(location = 1) in  vec3 vertexColor;"
		"layout(location = 0) out vec3 outColor;"

		"uniform mat4 modelMatrix;"

		"void main() {"
		"  gl_Position = modelMatrix * vec4(vertexPosition, 1.0);"
		"  outColor = vertexColor;"
		"}";

	GLuint vs = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vs, 1, &vertex_shader, NULL);
	glCompileShader(vs);

	glGetShaderInfoLog(vs, 512, &errLen, err);
	if (*err) printf("%s\n", err);

	const char* fragment_shader =
		"#version 410\n"

		"layout(location = 0) in  vec3 fragmentColor;"
		"layout(location = 0) out vec4 color;"

		"void main() {"
		"  color = vec4(1.0, 1.0, 1.0, 1.0);"
		"  color = vec4(fragmentColor, 1.0);"
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

	using namespace Mat;
	GLuint modelMatrixID = glGetUniformLocation(glProgram, "modelMatrix");

	mat4 model = mat4::Identity();
	mat4 view = lookAt(vec3{ 1, 0, -3 }, vec3{ 0, 0, 0 }, vec3{ 0, 1, 0 });
	constexpr mat4 proj = ortho(-1.5, 1.5, -1.5, 1.5, -10, 10);
	mat4 pmvMatrix = proj * view * model;

	glUniformMatrix4fv(modelMatrixID, 1, true, *pmvMatrix.data);

	mesh = new GMesh("res/suzanne.glb");

	return true;
}

void Engine::deinit()
{
	audioEngine.deinit();
	SDL_DestroyWindow(sdlWindow);
	SDL_Quit();

	sdlWindow = nullptr;

	delete mesh;
	mesh = nullptr;
}

void Engine::render()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	mesh->draw();
	SDL_GL_SwapWindow(sdlWindow);
}
