#pragma once

#include <vector>

#include "renderer/core/GLContext.hpp"
#include "renderer/core/Camera.hpp"
#include "renderer/Renderer.hpp"
#include "input/InputHandler.hpp"
#include "simulation/environment/Terrain.hpp"
#include "simulation/environment/ProceduralTerrain.hpp"
#include "simulation/environment/SRTMTerrain.hpp"
#include "simulation/core/RigidBodyState.hpp"
#include "simulation/physics/BallisticsModel.hpp"
#include "simulation/launchers/Launcher.hpp"
#include "simulation/projectiles/Projectile.hpp"
#include "simulation/core/Integrator.hpp"

#include <nlohmann/json.hpp>
#include <memory>
using json = nlohmann::json;


class Application {
public:
	bool init();
	void run();
	
	private:
	void handleInput();
	void render();
	
	void	initializeDearGUI();
	void	updateDearGUI();
	
	struct CompatibleProjectile {
		std::string filename;
		double      muzzleVelocity;
	};

	std::vector<std::string>		loadLauncherCatalog();
	Launcher						loadLauncherFromJson(const std::string& fileName);
	Projectile						loadProjectileFromJson(const std::string& fileName);
	std::vector<std::string>		loadTerrainCatalog();
	void							switchTerrain(const std::string& path);

	DragTable	loadDragTable(); // Currently hardcoded, will be loaded from JSON in Phase 3.
	void	iterateProjectilesTrajectories(double dt);
	void	importTrajectoryTableToCSV(const Trajectory& t);


	glm::dvec3	rayFromMouce(float screenX, float screenY);
	void		setLauncherToMap(Launcher& l);

	GLContext					m_context;
	Camera						m_camera;
	Renderer					m_renderer;
	InputHandler				m_input;
	std::unique_ptr<Terrain>	m_terrain;

	
	bool						m_instantFire     = true;
	int							m_csvSeparator    = 1;    // 0=comma, 1=semicolon, 2=tab
	std::vector<std::string>		m_terrainCatalog;         // full paths to .hgt files
	int								m_selectedTerrain = -1;   // -1 = procedural
	std::vector<std::string>			m_launcherCatalog;
	int									m_selectedLauncher  = 0;
	std::vector<CompatibleProjectile>	m_compatibleProjectiles;
	int									m_selectedProjectile = 0;
	Launcher					m_launcher	= Launcher({0.0,0.0,0.0}, 0.0, 45.0, 100.0);
	Projectile					m_projectile = Projectile(45, 0.155, 0.3, 3.937007874, 1.7);
	std::vector<Projectile>		m_listOfProjectiles;
	Trajectory					m_trajectory;
	std::vector<Trajectory>		m_listOfTrajectories;
	
	Wind	m_wind;

	bool m_running = false;
};
