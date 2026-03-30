#pragma once

#include <vector>
#include <future>

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
#include "simulation/fire_control/FireSolutionSolver.hpp"

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
	void	updateLauncherManagerGUI();
	
	struct ChargeLevel {
		std::string name;
		double      muzzleVelocity;
	};

	struct CompatibleProjectile {
		std::string            filename;
		std::vector<ChargeLevel> charges;
	};

	std::vector<std::string>		loadLauncherCatalog();
	Launcher						loadLauncherFromJson(const std::string& fileName);
	Projectile						loadProjectileFromJson(const std::string& fileName, double latitudeRad = 1.04719755);
	std::vector<std::string>		loadTerrainCatalog();
	void							switchTerrain(const std::string& path);

	DragTable	loadDragTable(); // Currently hardcoded, will be loaded from JSON in Phase 3.
	void	iterateProjectilesTrajectories(double dt);
	void	exportTrajectoryTableToCSV(const Trajectory& t);


	glm::dvec3	rayFromMouse(float screenX, float screenY);
	glm::dvec3	getPositionOnMap(double mousePosX, double mousePosY);
	void		setLauncherToMap(Launcher& l);

	void		saveScenario(const std::string& path);
	void		loadScenario(const std::string& path);

	void setMRSIinMotion();
	void fireControlMRSI(double dt);


	GLContext					m_context;
	Camera						m_camera;
	Renderer					m_renderer;
	InputHandler				m_input;
	std::unique_ptr<Terrain>	m_terrain;

	
	bool								m_instantFire     = true;
	int									m_csvSeparator    = 1;    // 0=comma, 1=semicolon, 2=tab
	std::vector<std::string>			m_terrainCatalog;         // full paths to .hgt files
	int									m_selectedTerrain = -1;   // -1 = procedural
	std::vector<std::string>			m_launcherCatalog;
	std::vector<CompatibleProjectile>	m_compatibleProjectiles;
	int									m_selectedProjectile = 0;
	int									m_selectedCharge     = 0;
	bool								m_mrsiLongestTof     = true;
	std::vector<Launcher>				m_launcher;
	std::vector<Projectile>				m_launcherProjectile; // one projectile per launcher
	std::vector<bool>					m_launcherSelected;
	int									m_placementQueueIdx = 0;
	Projectile							m_projectile = Projectile(45, 0.155, 0.3, 3.937007874, 1.7, 1.04719755);
	std::vector<Projectile>				m_listOfProjectiles;
	Trajectory							m_trajectory;
	std::vector<Trajectory>				m_listOfTrajectories;
	std::vector<double>					m_trajectoryAzimuths;
	std::vector<std::string>			m_trajectoryLabels;
	std::vector<SolvedFireSolutions>	m_solvedFireSolutions;
	
	Wind	m_wind;

	bool m_running = false;

	int									m_MRSI_burstSize = 1;
	std::vector<int>					m_MRSI_launcherBurstLeft;
	std::vector<std::vector<bool>>		m_MRSI_ProjectileLaunched;
	std::vector<double>					m_MRSI_launcherNextFireTime;
	double								m_MRSI_timer = 0;
	double								m_MRSI_secuanceLength;
	bool								m_MRSI_onGoing	= false;
};
