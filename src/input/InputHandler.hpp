#pragma once

#include "InputState.hpp"
#include <SDL3/SDL.h>

class InputHandler {
public:
	// Drain the SDL event queue and update InputState.
	// Pass the window so ImGui_ImplSDL3_ProcessEvent gets every event first.
	void poll(SDL_Window* window);

	const InputState& state() const { return m_state; }

private:
	InputState m_state;
};
