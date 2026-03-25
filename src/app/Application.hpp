#pragma once

#include <vector>

#include "renderer/core/GLContext.hpp"
#include "renderer/core/Camera.hpp"
#include "renderer/Renderer.hpp"
#include "input/InputHandler.hpp"
#include "simulation/environment/ProceduralTerrain.hpp"
#include "simulation/core/RigidBodyState.hpp"
#include "simulation/physics/BallisticsModel.hpp"
#include "simulation/launchers/Launcher.hpp"
#include "simulation/projectiles/Projectile.hpp"
#include "simulation/core/Integrator.hpp"


class Application {
public:
	bool init();
	void run();

private:
	void handleInput();
	void render();

	void	initializeDearGUI();
	void	updateDearGUI();
	
	std::shared_ptr<std::vector<dragCdTableEntry>> loadDragTable(); // Curently hardcoded table.
	
	GLContext			m_context;
	Camera				m_camera;
	Renderer			m_renderer;
	InputHandler		m_input;
	ProceduralTerrain	m_terrain;

	
	Launcher					m_launcher	= Launcher({0.0,0.0,0.0}, 0.0, 45.0, 100.0);
	Projectile					m_projectile = Projectile(45, 0.155, 0.3, 3.937007874, 1.7);
	std::vector<RigidBodyState>	m_trajectory;
	
	Wind	m_wind;

	bool m_running = false;
};
