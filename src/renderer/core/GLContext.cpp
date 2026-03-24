#include "GLContext.hpp"

#include <glad/gl.h>
#include <iostream>

GLContext::~GLContext() {
	if (m_glContext) SDL_GL_DestroyContext(m_glContext);
	if (m_window)    SDL_DestroyWindow(m_window);
	SDL_Quit();
}

bool GLContext::init(const std::string& title, int width, int height) {
	if (!SDL_Init(SDL_INIT_VIDEO)) {
		std::cerr << "SDL_Init failed: " << SDL_GetError() << "\n";
		return false;
	}

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

	m_window = SDL_CreateWindow(
		title.c_str(), width, height,
		SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE
	);
	if (!m_window) {
		std::cerr << "SDL_CreateWindow failed: " << SDL_GetError() << "\n";
		return false;
	}

	m_glContext = SDL_GL_CreateContext(m_window);
	if (!m_glContext) {
		std::cerr << "SDL_GL_CreateContext failed: " << SDL_GetError() << "\n";
		return false;
	}

	if (!gladLoadGL(reinterpret_cast<GLADloadfunc>(SDL_GL_GetProcAddress))) {
		std::cerr << "GLAD failed to load OpenGL functions\n";
		return false;
	}

	SDL_GL_SetSwapInterval(1); // vsync on

	std::cout << "OpenGL  : " << glGetString(GL_VERSION)  << "\n";
	std::cout << "Renderer: " << glGetString(GL_RENDERER) << "\n";
	std::cout << "GLSL    : " << glGetString(GL_SHADING_LANGUAGE_VERSION) << "\n";

	return true;
}

void GLContext::swapBuffers() {
	SDL_GL_SwapWindow(m_window);
}
