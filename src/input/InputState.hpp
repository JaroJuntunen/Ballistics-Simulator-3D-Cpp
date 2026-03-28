#pragma once

// Plain data — no SDL types, no includes.
// Populated each frame by InputHandler, read by Application.
struct InputState {
	// Mouse movement (only non-zero while the corresponding button is held)
	float mouseDeltaX = 0.0f;
	float mouseDeltaY = 0.0f;
	float scrollY     = 0.0f;

	bool mouseLeft  = false;
	bool mouseRight = false;

	// Keys (true for one frame on press)
	bool space      = false;
	bool shiftHeld  = false;
	bool keyC       = false;
	bool keyR       = false;
	bool keyM		= false;
	bool keyT		= false;

	bool quit = false;

	float mousePosX, mousePosY;


};
