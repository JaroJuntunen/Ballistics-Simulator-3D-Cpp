#include "InputHandler.hpp"
#include <imgui_impl_sdl3.h>

void InputHandler::poll(SDL_Window* window) {
	(void)window;

	// Reset per-frame fields
	m_state.mouseDeltaX = 0.0f;
	m_state.mouseDeltaY = 0.0f;
	m_state.scrollY     = 0.0f;
	m_state.space       = false;
	m_state.keyC        = false;
	m_state.keyR        = false;
	m_state.keyM		= false;

	SDL_Event event;
	while (SDL_PollEvent(&event)) {
		ImGui_ImplSDL3_ProcessEvent(&event);

		switch (event.type) {
		case SDL_EVENT_QUIT:
			m_state.quit = true;
			break;

		case SDL_EVENT_MOUSE_BUTTON_DOWN:
			if (event.button.button == SDL_BUTTON_LEFT)  m_state.mouseLeft  = true;
			if (event.button.button == SDL_BUTTON_RIGHT) m_state.mouseRight = true;
			break;

		case SDL_EVENT_MOUSE_BUTTON_UP:
			if (event.button.button == SDL_BUTTON_LEFT)  m_state.mouseLeft  = false;
			if (event.button.button == SDL_BUTTON_RIGHT) m_state.mouseRight = false;
			break;

		case SDL_EVENT_MOUSE_MOTION:
			m_state.mouseDeltaX = event.motion.xrel;
			m_state.mouseDeltaY = event.motion.yrel;
			break;

		case SDL_EVENT_MOUSE_WHEEL:
			m_state.scrollY = event.wheel.y;
			break;

		case SDL_EVENT_KEY_DOWN: {
			SDL_Keymod mods = SDL_GetModState();
			m_state.shiftHeld = (mods & SDL_KMOD_SHIFT) != 0;

			switch (event.key.key) {
			case SDLK_ESCAPE:	m_state.quit	= true; break;
			case SDLK_SPACE:	m_state.space	= true; break;
			case SDLK_C:		m_state.keyC	= true; break;
			case SDLK_R:		m_state.keyR	= true; break;
			case SDLK_M:
				m_state.keyM	= true;
				SDL_GetMouseState(&m_state.moucePosX, &m_state.moucePosY);
				break;
			default: break;
			}
			break;
		}

		case SDL_EVENT_KEY_UP:
			if (event.key.key == SDLK_LSHIFT || event.key.key == SDLK_RSHIFT)
				m_state.shiftHeld = false;
			break;

		default:
			break;
		}
	}
}
