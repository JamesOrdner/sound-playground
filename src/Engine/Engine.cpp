#include "Engine.h"
#include "../Graphics/Matrix.h"
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
	GLfloat points[] = {
		 0.5f, -0.5f,  0.f,
		 0.5f,  0.5f,  0.f,
		-0.5f, -0.5f,  0.f,
		-0.5f,  0.5f,  0.f
	};

	GLfloat colors[] = {
		0.583f,  0.771f,  0.014f,
		0.609f,  0.115f,  0.436f,
		0.327f,  0.483f,  0.844f,
		0.822f,  0.569f,  0.201f
	};

	GLuint indices[] = {
		0, 1, 3,
		0, 2, 3
	};

	GLuint vbo[3]; // [vertex, indices]
	glGenBuffers(3, vbo);

	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	// Vertex buffer
	glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
	glBufferData(GL_ARRAY_BUFFER, 12 * sizeof(GLfloat), points, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glEnableVertexAttribArray(0);

	// Vertex buffer
	glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
	glBufferData(GL_ARRAY_BUFFER, 12 * sizeof(GLfloat), colors, GL_STATIC_DRAW);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glEnableVertexAttribArray(1);

	// Index buffer
	indexBuffer = vbo[2];
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo[2]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, 6 * sizeof(GLuint), indices, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	glBindVertexArray(0);

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

	GLuint modelMatrixID = glGetUniformLocation(glProgram, "modelMatrix");
	Mat::mat4 model = Mat::mat4::Identity();
	Mat::mat4 view = Mat::lookAt(Mat::vec3{ 1, 0, 1 }, Mat::vec3{ 0, 0, 0 }, Mat::vec3{ 0, 1, 0 });
	Mat::mat4 proj = Mat::ortho(-1, 1, -1, 1, -20, 20);
	Mat::mat4 pmvMatrix = proj * view * model;
	glUniformMatrix4fv(modelMatrixID, 1, true, *pmvMatrix.data);

	return true;
}

void Engine::deinit()
{
	audioEngine.deinit();
	SDL_DestroyWindow(sdlWindow);
	SDL_Quit();

	sdlWindow = nullptr;
}

void Engine::render()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glBindVertexArray(vao);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	SDL_GL_SwapWindow(sdlWindow);
}
