#include "Render.h"
#include "GProgram.h"
#include "GMesh.h"
#include "GTexture.h"
#include "../UI/UIObject.h"
#include <SDL.h>
#include <GL/gl3w.h>
#include <stdio.h>

constexpr int glMajorVersion = 4;
constexpr int glMinorVersion = 6;

Render::Render() :
	glContext(nullptr),
	shadowFBO(0),
	shadowTexture(0),
	uiVAO(0)
{
}

Render::~Render() = default;

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
	glDepthFunc(GL_LESS);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	programMain = std::make_unique<GProgram>("main");
	programMain->setMat4Uniform("viewProj", projectionViewMatrix(window));
	programMain->setPreDrawRoutine([window] {
		int width, height;
		SDL_GL_GetDrawableSize(window, &width, &height);
		glViewport(0, 0, width, height);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		});

	if (!initShadow()) {
		printf("Failed to initialize shadow program.\n");
		programMain.reset();
		SDL_GL_DeleteContext(glContext);
		glContext = nullptr;
		return false;
	}

	if (!initUI()) {
		printf("Failed to initialize UI program.\n");
		programMain.reset();
		programShadow.reset();
		SDL_GL_DeleteContext(glContext);
		glContext = nullptr;
		return false;
	}

	uiTexture = std::make_unique<GTexture>("testTex");

	mat::mat4 pv = projectionViewMatrix(window);
	invProjectionViewMatrix = mat::inverse(pv);

	return true;
}

void Render::deinit()
{
	// programs
	programMain.reset();
	programShadow.reset();

	// framebuffers
	glDeleteFramebuffers(1, &shadowFBO);

	// textures
	glDeleteTextures(1, &shadowTexture);
	
	// ui
	glDeleteVertexArrays(1, &uiVAO);

	// context
	SDL_GL_DeleteContext(glContext);
	glContext = nullptr;
}

void Render::setCamera(SDL_Window* window, const mat::vec3& position, const mat::vec3& focus)
{
	int width, height;
	SDL_GL_GetDrawableSize(window, &width, &height);
	float aspectRatio = static_cast<float>(height) / static_cast<float>(width);
	mat::mat4 view = lookAt(position, focus);
	mat::mat4 proj = mat::perspective(-0.05f, 0.05f, -0.05f * aspectRatio, 0.05f * aspectRatio, 0.1f, 15.f);
	mat::mat4 projView = proj * view;
	programMain->setMat4Uniform("viewProj", projView);
	invProjectionViewMatrix = mat::inverse(projView);
}

void Render::draw(const std::map<std::string, std::weak_ptr<GMesh>>& meshes)
{
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);

	// Synchronize transform data
	for (const auto& mesh : meshes) mesh.second.lock()->updateInstanceTransforms();

	// Render shadow buffer
	programShadow->use();
	for (const auto& mesh : meshes) mesh.second.lock()->draw();
	programShadow->release();

	// Render main program
	programMain->use();
	for (const auto& mesh : meshes) mesh.second.lock()->draw();
}

void Render::drawUI(SDL_Window* window, const UIObject& rootObject)
{
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBindVertexArray(uiVAO);
	glBindTexture(GL_TEXTURE_2D, uiTexture->id());
	programUI->use();

	int x, y;
	SDL_GL_GetDrawableSize(window, &x, &y);
	mat::vec2 parentCoords{ 0, 0 };
	mat::vec2 screenBounds{ static_cast<float>(x), static_cast<float>(y) };
	
	drawUIRecursive(rootObject, parentCoords, 1.f, screenBounds);

	glBindTexture(GL_TEXTURE_2D, 0);
}

void Render::drawUIRecursive(
	const UIObject& object,
	const mat::vec2& parentCoords,
	float parentScale,
	const mat::vec2& screenBounds)
{
	mat::vec2 scale = object.bounds / screenBounds * parentScale * object.scale;
	mat::vec2 translation = parentCoords + object.position;
	translation -= object.anchorPosition() * scale;

	mat::mat3 transform{
		{ scale.x,       0, translation.x },
		{       0, scale.y, translation.y },
		{       0,       0,             1 }
	};

	programUI->setMat3Uniform("transform", transform);
	glDrawArrays(GL_TRIANGLES, 0, 6);

	for (const auto& child : object.subobjects) {
		drawUIRecursive(child, translation, parentScale * object.scale, screenBounds);
	}
}

void Render::show(SDL_Window* window)
{
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

	programShadow = std::make_unique<GProgram>("shadow");
	programShadow->setFramebuffer(shadowFBO);
	programShadow->setPreDrawRoutine([shadowMap_x, shadowMap_y, shadowTexture = shadowTexture] {
		glClear(GL_DEPTH_BUFFER_BIT);
		glViewport(0, 0, shadowMap_x, shadowMap_y);
		glEnable(GL_POLYGON_OFFSET_FILL);
		glBindTexture(GL_TEXTURE_2D, shadowTexture);
		});
	programShadow->setReleaseRoutine([] {
		glDisable(GL_POLYGON_OFFSET_FILL);
		});

	mat::mat4 view = lookAt(mat::vec3{ 2.f, 3.f, -0.5f }, mat::vec3{ 0.f, 0.f, 0.f });
	mat::mat4 proj = mat::ortho(-3.f, 3.f, -3.f, 3.f, -10.f, 10.f);
	mat::mat4 projectionViewMatrix = proj * view;
	programShadow->setMat4Uniform("mvp", projectionViewMatrix);

	mat::mat4 biasMatrix{
		{ 0.5, 0.0, 0.0, 0.5 },
		{ 0.0, 0.5, 0.0, 0.5 },
		{ 0.0, 0.0, 0.5, 0.5 },
		{ 0.0, 0.0, 0.0, 1.0 }
	};
	mat::mat4 depthBiasMVP = biasMatrix * projectionViewMatrix;
	programMain->setMat4Uniform("shadowMVP", depthBiasMVP);

	return true;
}

bool Render::initUI()
{
	static const GLfloat g_vertex_data[] = {
		-1.f, -1.f, // LL
		-1.f,  1.f, // UL
		 1.f,  1.f, // UR
		-1.f, -1.f, // LL
		 1.f, -1.f, // LR
		 1.f,  1.f, // UR
	};

	static const GLfloat g_texcoord_data[] = {
		0.f, 0.f, // LL
		0.f, 1.f, // UL
		1.f, 1.f, // UR
		0.f, 0.f, // LL
		1.f, 0.f, // LR
		1.f, 1.f, // UR
	};

	glGenVertexArrays(1, &uiVAO);
	glBindVertexArray(uiVAO);

	GLuint vbo;
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glEnableVertexAttribArray(0);
	glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_data), g_vertex_data, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);

	GLuint vbo_texcoord;
	glGenBuffers(1, &vbo_texcoord);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_texcoord);
	glEnableVertexAttribArray(1);
	glBufferData(GL_ARRAY_BUFFER, sizeof(g_texcoord_data), g_texcoord_data, GL_STATIC_DRAW);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);
	
	glBindVertexArray(0);

	glDeleteBuffers(1, &vbo);
	glDeleteBuffers(1, &vbo_texcoord);

	programUI = std::make_unique<GProgram>("ui");

	return true;
}
