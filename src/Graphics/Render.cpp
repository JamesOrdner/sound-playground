#include "Render.h"
#include "GProgram.h"
#include "GMesh.h"
#include <SDL.h>
#include <GL/gl3w.h>
#include <stdio.h>

constexpr int glMajorVersion = 4;
constexpr int glMinorVersion = 6;

Render::Render() :
	glContext(nullptr),
	shadowFBO(0),
	shadowTexture(0)
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

	program_main = std::make_shared<GProgram>("main");
	program_main->setMatrixUniform("viewProj", projectionViewMatrix(window));
	program_main->setPreDrawRoutine([window] {
		int width, height;
		SDL_GL_GetDrawableSize(window, &width, &height);
		glViewport(0, 0, width, height);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		});

	if (!initShadow()) {
		printf("Failed to initialize shadow program.\n");
		program_main.reset();
		SDL_GL_DeleteContext(glContext);
		glContext = nullptr;
		return false;
	}

	mat::mat4 pv = projectionViewMatrix(window);
	invProjectionViewMatrix = mat::inverse(pv);

	return true;
}

void Render::deinit()
{
	// programs
	program_main.reset();
	program_shadow.reset();

	// framebuffers
	glDeleteFramebuffers(1, &shadowFBO);

	// textures
	glDeleteTextures(1, &shadowTexture);

	// context
	SDL_GL_DeleteContext(glContext);
	glContext = nullptr;
}

void Render::draw(SDL_Window* window, const std::map<std::string, std::weak_ptr<GMesh>>& meshes)
{
	// Synchronize transform data
	for (const auto& mesh : meshes) mesh.second.lock()->updateInstanceTransforms();

	// Render shadow buffer
	program_shadow->use();
	for (const auto& mesh : meshes) mesh.second.lock()->draw();
	program_shadow->release();

	// Render main program
	program_main->use();
	for (const auto& mesh : meshes) mesh.second.lock()->draw();

	SDL_GL_SwapWindow(window);
}

const mat::mat4& Render::screenToWorldMatrix()
{
	return invProjectionViewMatrix;
}

mat::mat4 Render::projectionViewMatrix(SDL_Window* window)
{
	int width, height;
	SDL_GL_GetDrawableSize(window, &width, &height);
	float aspectRatio = static_cast<float>(height) / static_cast<float>(width);
	mat::mat4 view = lookAt(mat::vec3{ 0.f, 10.f, 4.f }, mat::vec3{ 0.f, 0.f, 0.f });
	mat::mat4 proj = mat::ortho(-3.5f, 3.5f, -3.5f * aspectRatio, 3.5f * aspectRatio, -50.f, 0.f);
	return proj * view;
}

bool Render::initShadow()
{
	constexpr int shadowMap_x = 2048;
	constexpr int shadowMap_y = 2048;

	glGenFramebuffers(1, &shadowFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, shadowFBO);

	glGenTextures(1, &shadowTexture);
	glBindTexture(GL_TEXTURE_2D, shadowTexture);
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_DEPTH_COMPONENT32, shadowMap_x, shadowMap_y);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, shadowTexture, 0);
	glBindTextureUnit(0, shadowTexture);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		glDeleteFramebuffers(1, &shadowFBO);
		glDeleteTextures(1, &shadowTexture);
		return false;
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glPolygonOffset(2.f, 2.f);

	program_shadow = std::make_shared<GProgram>("shadow");
	program_shadow->setFramebuffer(shadowFBO);
	program_shadow->setPreDrawRoutine([shadowMap_x, shadowMap_y] {
		glClear(GL_DEPTH_BUFFER_BIT);
		glViewport(0, 0, shadowMap_x, shadowMap_y);
		glEnable(GL_POLYGON_OFFSET_FILL);
		});
	program_shadow->setReleaseRoutine([] {
		glDisable(GL_POLYGON_OFFSET_FILL);
		});

	mat::mat4 view = lookAt(mat::vec3{ 2.f, 3.f, -0.5f }, mat::vec3{ 0.f, 0.f, 0.f });
	mat::mat4 proj = mat::ortho(-3.f, 3.f, -3.f, 3.f, -10.f, 10.f);
	mat::mat4 projectionViewMatrix = proj * view;
	program_shadow->setMatrixUniform("mvp", projectionViewMatrix);

	mat::mat4 biasMatrix{
		{ 0.5, 0.0, 0.0, 0.5 },
		{ 0.0, 0.5, 0.0, 0.5 },
		{ 0.0, 0.0, 0.5, 0.5 },
		{ 0.0, 0.0, 0.0, 1.0 }
	};
	mat::mat4 depthBiasMVP = biasMatrix * projectionViewMatrix;
	program_main->setMatrixUniform("shadowMVP", depthBiasMVP);

	return true;
}
