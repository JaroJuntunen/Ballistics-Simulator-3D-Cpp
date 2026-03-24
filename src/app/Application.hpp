#pragma once

#include "renderer/core/GLContext.hpp"
#include "renderer/core/Camera.hpp"
#include "renderer/Renderer.hpp"
#include "input/InputHandler.hpp"
#include "simulation/environment/ProceduralTerrain.hpp"

class Application {
public:
    bool init();
    void run();

private:
    void handleInput();
    void render();

    GLContext          m_context;
    Camera             m_camera;
    Renderer           m_renderer;
    InputHandler       m_input;
    ProceduralTerrain  m_terrain;

    bool m_running = false;
};
