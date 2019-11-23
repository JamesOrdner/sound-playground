#include "Engine.h"
#include "../Graphics/Matrix.h"
#include "../Graphics/GMesh.h"
#include "EModel.h"

#include <SDL.h>
#include <GL/gl3w.h>
#include <stdio.h>

Engine& Engine::instance()
{
	static Engine instance;
	return instance;
}

EWorld& Engine::world()
{
	return *_world;
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
			case SDL_MOUSEBUTTONDOWN:
				mat::vec3 hitLoc;
				std::shared_ptr<EModel> hitObject;
				if (_world->raycast(mat::vec3{ 0.f, 1.f, 0.f }, mat::vec3{ 0.f, -1.f, 0.f }, hitLoc, hitObject)) {
					printf("%f %f %f\n", hitLoc.x, hitLoc.y, hitLoc.z);
				}
			}
		}

		render();
	}
}

void Engine::registerModel(const std::shared_ptr<EModel>& model)
{
	// Assign mesh to model and save model reference
	auto mesh = makeMesh(model->getFilepath());
	model->registerWithMesh(mesh); // transfer mesh ownership to model
	mesh->registerModel(model); // add weak pointer to model in mesh model list
}

void Engine::unregisterModel(const std::shared_ptr<EModel>& model)
{
	auto mesh = model->getMesh();
	mesh->unregisterModel(model); // remove model from mesh model list

	// Remove mesh from map if no more models reference this mesh
	std::string path = model->getFilepath();
	auto meshRef = meshes[path];
	if (meshRef.expired()) meshes.erase(path);

	model->unregister();
}

std::shared_ptr<GMesh> Engine::makeMesh(const std::string& filepath)
{
	if (auto existing = meshes[filepath].lock()) {
		return existing;
	}
	else {
		auto newMesh = std::make_shared<GMesh>(filepath);
		meshes[filepath] = newMesh;
		return newMesh;
	}
}

Engine::Engine()
{
	if (init()) {
		_world = std::make_unique<EWorld>();
		bInitialized = true;
	}
	else {
		bInitialized = false;
	}
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

	SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
	SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 4);

	// Create window
	sdlWindow = SDL_CreateWindow("Sound Playground", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 768, 768, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);
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
		"layout(location = 3) in  vec3 instancePosition;"
		"layout(location = 4) in  vec3 instanceScale;"

		"layout(location = 0) out vec3 outNormal;"
		"layout(location = 1) out vec3 outTexCoord;"

		"uniform mat4 viewProj;"

		"void main() {"
		"  gl_Position = viewProj * vec4(position * instanceScale + instancePosition, 1.0);"
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

	GLuint viewProjMatrixID = glGetUniformLocation(glProgram, "viewProj");
	mat::mat4 view = lookAt(mat::vec3{ 1, 1, 1 }, mat::vec3());
	mat::mat4 proj = mat::ortho(-1.5, 1.5, -1.5, 1.5, -10, 10);
	mat::mat4 mvp = proj * view;
	glUniformMatrix4fv(viewProjMatrixID, 1, true, *mvp.data);

	return true;
}

void Engine::deinit()
{
	bInitialized = false;

	audioEngine.deinit();

	SDL_DestroyWindow(sdlWindow);
	sdlWindow = nullptr;

	SDL_Quit();
}

void Engine::render()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	for (const auto& mesh : meshes) mesh.second.lock()->draw();
	SDL_GL_SwapWindow(sdlWindow);
}
