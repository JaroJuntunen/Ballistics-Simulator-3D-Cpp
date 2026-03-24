#pragma once

#include <SDL3/SDL.h>
#include <string>

class GLContext {
public:
	~GLContext();

	bool init(const std::string& title, int width, int height);
	void swapBuffers();

	SDL_Window*   window()    const { return m_window; }
	SDL_GLContext glContext() const { return m_glContext; }

private:
	SDL_Window*   m_window    = nullptr;
	SDL_GLContext m_glContext = nullptr;
};
