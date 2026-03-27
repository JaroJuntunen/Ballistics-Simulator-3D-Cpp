#include "Application.hpp"

#include <SDL3/SDL.h>
#include <imgui.h>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_opengl3.h>
#include <iostream>
#include <fstream>
#include <filesystem>

bool Application::init() {
	if (!m_context.init("Ballistics Simulator 3D", 1600, 900))
		return false;

	m_terrain = std::make_unique<ProceduralTerrain>();
	m_terrainCatalog = loadTerrainCatalog();

	m_camera.setOrbit(45.0f, 30.0f, 1800.0f);
	m_renderer.initTerrain(*m_terrain, (int)(m_terrain->width() / 100.0f));
	m_renderer.initTrajectory();
	double groundZ = m_terrain->heightAt(0.0f, 0.0f);
	m_launcherCatalog = loadLauncherCatalog();
	if (!m_launcherCatalog.empty()) {
		m_launcher = loadLauncherFromJson(m_launcherCatalog[0]);
		m_launcher.setPosition({0.0, 0.0, groundZ + 1.0});
		if (!m_compatibleProjectiles.empty()) {
			m_projectile = loadProjectileFromJson(m_compatibleProjectiles[0].filename);
			m_launcher.setSpeed(m_compatibleProjectiles[0].muzzleVelocity);
		}
	} else {
		m_launcher = Launcher({0.0, 0.0, groundZ + 1.0f}, 0.0, 45.0, 900.0);
		m_projectile = Projectile(48, 0.155, 0.3, 0.322580645, 1.2);
		m_projectile.setDragTable(loadDragTable());
	}
	m_running = true;
	initializeDearGUI();
	return true;
}

void Application::run() {
	Uint64 prev  = SDL_GetPerformanceCounter();
	float  freq  = static_cast<float>(SDL_GetPerformanceFrequency());

	while (m_running) {
		Uint64 now = SDL_GetPerformanceCounter();
		float  dt  = static_cast<float>(now - prev) / freq;
		prev = now;

		m_wind.passWindTime(dt);
		m_input.poll(m_context.window());
		if (m_input.state().quit)
		m_running = false;
		
		updateDearGUI();
		handleInput();
		iterateProjectilesTrajectories(dt);
		render();
	}

	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplSDL3_Shutdown();
	ImGui::DestroyContext();
}

void Application::iterateProjectilesTrajectories(double dt)
{
	for(int i = 0; i < m_listOfProjectiles.size(); i++){
		Projectile& proj = m_listOfProjectiles.at(i);
		Trajectory& traj = m_listOfTrajectories.at(i);

		if (!proj.getIsImpacted()) {
			traj.push_back(Integrator::step(traj.back(), proj, m_wind,m_launcher.getLatitudeInRad(), dt, BallisticsModel::derivative));
			proj.setIsImpacted(BallisticsModel::hasImpacted(traj.back(), *m_terrain));
			m_renderer.updateTrajectory(i, traj);
		}
	}
}

void Application::importTrajectoryTableToCSV(const Trajectory& t)
{
	std::filesystem::create_directories("Exports");
	std::string path = "Exports/trajectoryExport.csv";
	for (int i = 1; std::filesystem::exists(path); ++i)
		path = "Exports/trajectoryExport_" + std::to_string(i) + ".csv";
	std::ofstream file(path);
	if (!file.is_open()) return;
	const char sep = (m_csvSeparator == 1) ? ';' : (m_csvSeparator == 2) ? '\t' : ',';
	file << "Range (m)" << sep << "Height(m)" << sep << "Drift(m)" << sep << "Speed(m/s)" << sep << "TOF (s)\n";

	double azRad = glm::radians(m_launcher.getAzimuth());
	glm::dvec2 right = { std::sin(azRad), -std::cos(azRad) };
	const double	sampleInterval	= 500.0;
	const	glm::dvec3 origin		= t.front().position;
	const	double physDt			= 0.01;
	double	nextSample				= 0.0;
	int		index					= 0;
	for (const auto& state : t) {
		glm::dvec2 horiz = {
			state.position.x - origin.x,
			state.position.y - origin.y };
		double range = glm::length(horiz);
		if (range >= nextSample) {
			double height = state.position.z - origin.z;
			double drift  = glm::dot(horiz, right);
			double speed  = glm::length(state.velocity);
			double tof    = index * physDt;
			file << range << sep << height << sep << drift << sep << speed << sep << tof << "\n";
			nextSample += sampleInterval;
		}
		++index;
	}
	file.close();
}

glm::dvec3 Application::rayFromMouce(float screenX, float screenY)
{
	int w, h;
	SDL_GetWindowSizeInPixels(m_context.window(), &w, &h);

	// Convert pixel -> NDC [-1, 1]
	float ndcX = (2.0f * screenX / w) - 1.0f;
	float ndcY = 1.0f - (2.0f * screenY / h);  // flip Y — screen Y grows down

	// Unproject: clip space -> world space
	glm::mat4 invVP = glm::inverse(m_renderer.proj() * m_renderer.view());
	glm::vec4 nearPt = invVP * glm::vec4(ndcX, ndcY, -1.0f, 1.0f);
	glm::vec4 farPt  = invVP * glm::vec4(ndcX, ndcY,  1.0f, 1.0f);

	nearPt /= nearPt.w;
	farPt  /= farPt.w;

	return glm::normalize(glm::dvec3(farPt - nearPt));
}

void Application::setLauncherToMap(Launcher &l)
{
	glm::dvec3 rayOrigin = m_camera.position();
	glm::dvec3 rayDir = rayFromMouce(m_input.state().moucePosX,m_input.state().moucePosY);

	// March along ray in 10m steps
	for (double t = 0; t < 200000.0; t += 10.0) {
		glm::dvec3 pt = rayOrigin + rayDir * t;
		if (pt.z <= m_terrain->heightAt((float)pt.x, (float)pt.y)) {
			// Hit — place launcher here
			pt.z = m_terrain->heightAt((float)pt.x, (float)pt.y) + 1.0;
			m_launcher.setPosition(pt);
			break;
		}
	}
}

void Application::handleInput() {
	const InputState& in = m_input.state();
	const ImGuiIO&    io = ImGui::GetIO();

	// Camera controls
	if (io.WantCaptureMouse)
		return;

	if (in.mouseLeft)
		m_camera.orbit(-in.mouseDeltaX * 0.4f, in.mouseDeltaY * 0.4f);

	if (in.mouseRight)
		m_camera.pan(in.mouseDeltaX, in.mouseDeltaY);

	if (in.scrollY != 0.0f)
		m_camera.zoom(in.scrollY * -40.0f);

	// Fire controls
	if (in.space) {
		Projectile proj = m_projectile;
		RigidBodyState initialState = m_launcher.fire(proj);
		m_listOfProjectiles.push_back(proj);
		StopFn stop = [&](const RigidBodyState& s) {
			return BallisticsModel::hasImpacted(s, *m_terrain);
		};
		if (m_instantFire)
			m_trajectory = Integrator::simulateSteps(initialState, proj, m_wind, m_launcher.getLatitudeInRad(), 0.01, BallisticsModel::derivative, stop);
		else {
			m_trajectory.clear();
			m_trajectory.push_back(initialState);
		}
		m_listOfTrajectories.push_back(m_trajectory);
		m_renderer.addTrajectory(m_trajectory);
	}
	if (in.keyM) {
		setLauncherToMap(m_launcher);
	}


}

void Application::render() {
	int w, h;
	SDL_GetWindowSizeInPixels(m_context.window(), &w, &h);

	m_renderer.beginFrame(w, h, m_camera);
	m_renderer.endFrame();
	
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
	m_context.swapBuffers();

}

void Application::initializeDearGUI()
{
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui::StyleColorsDark();
	ImGui_ImplSDL3_InitForOpenGL(m_context.window(), m_context.glContext());
	ImGui_ImplOpenGL3_Init("#version 330 core");
}

void Application::updateDearGUI()
{
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplSDL3_NewFrame();
	ImGui::NewFrame();

	ImGui::SetNextWindowPos({10, 24}, ImGuiCond_FirstUseEver);
	ImGui::Begin("Simulator", nullptr, ImGuiWindowFlags_AlwaysAutoResize);


	if (ImGui::CollapsingHeader("Terrain")) {
		// Procedural option + all .hgt files
		const char* proceduralLabel = "Procedural (Perlin)";
		const char* currentLabel = (m_selectedTerrain == -1)
			? proceduralLabel
			: std::filesystem::path(m_terrainCatalog[m_selectedTerrain]).stem().string().c_str();

		if (ImGui::BeginCombo("Tile##terrain", currentLabel)) {
			// Procedural fallback entry
			if (ImGui::Selectable(proceduralLabel, m_selectedTerrain == -1)) {
				m_selectedTerrain = -1;
				switchTerrain("");
			}
			if (m_selectedTerrain == -1) ImGui::SetItemDefaultFocus();

			// One entry per .hgt file
			for (int i = 0; i < (int)m_terrainCatalog.size(); ++i) {
				std::string label = std::filesystem::path(m_terrainCatalog[i]).stem().string();
				bool selected = (m_selectedTerrain == i);
				if (ImGui::Selectable(label.c_str(), selected)) {
					m_selectedTerrain = i;
					switchTerrain(m_terrainCatalog[i]);
				}
				if (selected) ImGui::SetItemDefaultFocus();
			}
			ImGui::EndCombo();
		}

		if (m_selectedTerrain >= 0) {
			auto* srtm = dynamic_cast<SRTMTerrain*>(m_terrain.get());
			if (srtm) {
				ImGui::Text("Origin : %.4f N  %.4f E", srtm->originLat(), srtm->originLon());
				ImGui::Text("Width  : %.1f km", srtm->width()  / 1000.0f);
				ImGui::Text("Height : %.1f km", srtm->height() / 1000.0f);
			}
		}
	}

	if (ImGui::CollapsingHeader("Wind")) {
		glm::dvec3 base = m_wind.getBaseWindSpeed();
		float windX = (float)base.x;
		float windY = (float)base.y;
		float windZ = (float)base.z;
		float gustSeverity  = (float)m_wind.getWindGustSeverity();
		float gustFrequency = (float)m_wind.getGustFrequency();
		ImGui::SeparatorText("Base wind");
		ImGui::DragFloat("Wind X (m/s)", &windX, 0.1f, -100.0f, 100.0f);
		ImGui::DragFloat("Wind Y (m/s)", &windY, 0.1f, -100.0f, 100.0f);
		ImGui::DragFloat("Wind Z (m/s)", &windZ, 0.1f,  -50.0f,  50.0f);
		ImGui::SeparatorText("Gusts");
		ImGui::DragFloat("Gust severity",  &gustSeverity,  0.1f,  0.0f, 50.0f);
		ImGui::DragFloat("Gust frequency", &gustFrequency, 0.01f, 0.0f,  2.0f);
		m_wind.setBaseWindSpeed({(double)windX, (double)windY, (double)windZ});
		m_wind.setWindGustSeverity((double)gustSeverity);
		m_wind.setGustFrequency((double)gustFrequency);
	}
	
	if (ImGui::CollapsingHeader("Launcher")) {
		if (!m_launcherCatalog.empty()) {
			const std::string& current = m_launcherCatalog[m_selectedLauncher];
			if (ImGui::BeginCombo("Type##launcher", current.c_str())) {
				for (int i = 0; i < (int)m_launcherCatalog.size(); ++i) {
					bool selected = (m_selectedLauncher == i);
					if (ImGui::Selectable(m_launcherCatalog[i].c_str(), selected)) {
						m_selectedLauncher = i;
						m_launcher = loadLauncherFromJson(m_launcherCatalog[i]);
						glm::dvec3 pos = m_launcher.getPosition();
						pos.z = m_terrain->heightAt(pos.x, pos.y) + 1.0;
						m_launcher.setPosition(pos);
						if (!m_compatibleProjectiles.empty()) {
							m_projectile = loadProjectileFromJson(m_compatibleProjectiles[0].filename);
							m_launcher.setSpeed(m_compatibleProjectiles[0].muzzleVelocity);
						}
					}
					if (selected) ImGui::SetItemDefaultFocus();
				}
				ImGui::EndCombo();
			}
		}
		float azimuth   = (float)m_launcher.getAzimuth();
		float elevation = (float)m_launcher.getElevation();
		float speed     = (float)m_launcher.getSpeed();
		float latitude  = (float)m_launcher.getLatitude();
		ImGui::DragFloat("Azimuth (deg)",         &azimuth,   0.5f,  -180.0f, 180.0f);
		ImGui::DragFloat("Elevation (deg)",        &elevation, 0.5f,     0.0f,  90.0f);
		ImGui::DragFloat("Muzzle velocity (m/s)", &speed,     1.0f,     1.0f, 2000.0f);
		ImGui::DragFloat("Latitude (deg)",         &latitude,  0.5f,   -90.0f,  90.0f);
		m_launcher.setAzimuth((double)azimuth);
		m_launcher.setElevation((double)elevation);
		m_launcher.setSpeed((double)speed);
		m_launcher.setLatitude((double)latitude);
		ImGui::Checkbox("Instant trajectory", &m_instantFire);
	}

	if (ImGui::CollapsingHeader("Projectile")) {
		if (!m_compatibleProjectiles.empty()) {
			const std::string& current = m_compatibleProjectiles[m_selectedProjectile].filename;
			if (ImGui::BeginCombo("Type##projectile", current.c_str())) {
				for (int i = 0; i < (int)m_compatibleProjectiles.size(); ++i) {
					bool selected = (m_selectedProjectile == i);
					if (ImGui::Selectable(m_compatibleProjectiles[i].filename.c_str(), selected)) {
						m_selectedProjectile = i;
						m_projectile = loadProjectileFromJson(m_compatibleProjectiles[i].filename);
						m_launcher.setSpeed(m_compatibleProjectiles[i].muzzleVelocity);
					}
					if (selected) ImGui::SetItemDefaultFocus();
				}
				ImGui::EndCombo();
			}
		}
		float mass     = (float)m_projectile.getMass();
		float diameter = (float)m_projectile.getDiameter();
		float twist    = (float)m_projectile.getTwistRate();
		float sf       = (float)m_projectile.getStabilityFactor();
		ImGui::DragFloat("Mass (kg)",           &mass,     0.01f,  0.01f, 100.0f);
		ImGui::DragFloat("Diameter (m)",        &diameter, 0.001f, 0.001f,  0.5f);
		ImGui::DragFloat("Twist rate (rev/m)",  &twist,    0.01f,  0.01f,  10.0f);
		ImGui::DragFloat("Stability factor",    &sf,       0.01f,  0.5f,    5.0f);
		m_projectile.setMass((double)mass);
		m_projectile.setDiameter((double)diameter);
		m_projectile.setTwistRate((double)twist);
		m_projectile.setStabilityFactor((double)sf);
	}

	if (!m_listOfTrajectories.empty()) {
		const Trajectory& traj = m_listOfTrajectories.back();
		if (!traj.empty()) {
			ImGui::Separator();
			ImGui::Text("Trajectory points : %d", (int)traj.size());
			double dist = glm::distance(traj.back().position, traj.front().position);
			ImGui::Text("Impact distance   : %.1f m", dist);
		}

		if (ImGui::CollapsingHeader("Ballistic table")) {
			const Trajectory& t       = m_listOfTrajectories.back();
			const double sampleInterval = 500.0;
			const double physDt         = 0.01;

			double azRad = glm::radians(m_launcher.getAzimuth());
			glm::dvec2 right = { std::sin(azRad), -std::cos(azRad) };

			if (ImGui::BeginTable("balltable", 5,
				ImGuiTableFlags_Borders     |
				ImGuiTableFlags_RowBg       |
				ImGuiTableFlags_ScrollY,
				ImVec2(0.0f, 300.0f)))
			{
				ImGui::TableSetupScrollFreeze(0, 1);
				ImGui::TableSetupColumn("Range (m)");
				ImGui::TableSetupColumn("Height (m)");
				ImGui::TableSetupColumn("Drift (m)");
				ImGui::TableSetupColumn("Speed (m/s)");
				ImGui::TableSetupColumn("TOF (s)");
				ImGui::TableHeadersRow();

				if (!t.empty()) {
					const glm::dvec3 origin = t.front().position;
					double nextSample = 0.0;
					int    index      = 0;
					for (const auto& state : t) {
						glm::dvec2 horiz = {
							state.position.x - origin.x,
							state.position.y - origin.y };
						double range = glm::length(horiz);
						if (range >= nextSample) {
							double height = state.position.z - origin.z;
							double drift  = glm::dot(horiz, right);
							double speed  = glm::length(state.velocity);
							double tof    = index * physDt;
							ImGui::TableNextRow();
							ImGui::TableSetColumnIndex(0); ImGui::Text("%.0f",  range);
							ImGui::TableSetColumnIndex(1); ImGui::Text("%.1f",  height);
							ImGui::TableSetColumnIndex(2); ImGui::Text("%.2f",  drift);
							ImGui::TableSetColumnIndex(3); ImGui::Text("%.1f",  speed);
							ImGui::TableSetColumnIndex(4); ImGui::Text("%.2f",  tof);
							nextSample += sampleInterval;
						}
						++index;
					}
				}
				ImGui::EndTable();
			}
			static const char* separators[] = { "Comma (,)", "Semicolon (;)", "Tab (\\t)" };
			ImGui::SetNextItemWidth(130.0f);
			ImGui::Combo("##sep", &m_csvSeparator, separators, 3);
			ImGui::SameLine();
			if (ImGui::Button("Export CSV"))
				importTrajectoryTableToCSV(t);
		}
	}

	ImGui::End();
}

std::vector<std::string> Application::loadTerrainCatalog()
{
	std::vector<std::string> catalog;
	const std::string root = "data/terrain";
	if (!std::filesystem::exists(root)) return catalog;
	for (const auto& entry : std::filesystem::recursive_directory_iterator(root)) {
		if (entry.path().extension() == ".hgt")
			catalog.push_back(entry.path().string());
	}
	std::sort(catalog.begin(), catalog.end());
	return catalog;
}

void Application::switchTerrain(const std::string& path)
{
	if (path.empty()) {
		m_terrain = std::make_unique<ProceduralTerrain>();
		m_selectedTerrain = -1;
	} else {
		m_terrain = std::make_unique<SRTMTerrain>(path);
	}
	m_renderer.reloadTerrain(*m_terrain, (int)(m_terrain->width() / 100.0f));
	glm::dvec3 pos = m_launcher.getPosition();
	pos.z = m_terrain->heightAt((float)pos.x, (float)pos.y) + 1.0;
	m_launcher.setPosition(pos);
	auto* srtm = dynamic_cast<SRTMTerrain*>(m_terrain.get());
	if (srtm) {
		double latFraction = pos.y / srtm->height();
		m_launcher.setLatitude(srtm->originLat() + latFraction);
	}
}

std::vector<std::string> Application::loadLauncherCatalog()
{
	std::vector<std::string> launcherCatalog;
	const std::string path = "data/launchers";
	if (!std::filesystem::exists(path)) return launcherCatalog;
	for (const auto& entry : std::filesystem::directory_iterator(path)) {
		if (entry.path().extension() == ".json")
			launcherCatalog.push_back(entry.path().stem().string());
	}
	return launcherCatalog;
}
Launcher Application::loadLauncherFromJson(const std::string& fileName)
{
	std::ifstream file("data/launchers/" + fileName + ".json");
	if (!file.is_open()) return Launcher({0.0, 0.0, 0.0}, 0.0, 45.0, 900.0);
	json j = json::parse(file);

	glm::dvec3 position = {
		j["position"]["x"].get<double>(),
		j["position"]["y"].get<double>(),
		j["position"]["z"].get<double>()
	};
	double azimuth   = j["azimuth_deg"].get<double>();
	double elevation = j["elevation_deg"].get<double>();
	double latitude  = j["latitude_deg"].get<double>();

	m_compatibleProjectiles.clear();
	m_selectedProjectile = 0;
	for (const auto& cp : j["compatible_projectiles"]) {
		std::string name = cp["projectile"].get<std::string>();
		if (name.size() > 5 && name.substr(name.size() - 5) == ".json")
			name = name.substr(0, name.size() - 5);
		m_compatibleProjectiles.push_back({ name, cp["muzzle_velocity_ms"].get<double>() });
	}

	double speed = m_compatibleProjectiles.empty() ? 900.0 : m_compatibleProjectiles[0].muzzleVelocity;
	Launcher launcher(position, azimuth, elevation, speed);
	launcher.setLatitude(latitude);
	return launcher;
}

Projectile Application::loadProjectileFromJson(const std::string& fileName)
{
	std::ifstream file("data/projectiles/" + fileName + ".json");
	if (!file.is_open()) return Projectile(48, 0.155, 0.3, 0.322580645, 1.2);
	json j = json::parse(file);

	Projectile p(
		j["mass_kg"].get<double>(),
		j["diameter_m"].get<double>(),
		j["drag_coefficient_fallback"].get<double>(),
		j["twist_rate_rev_per_m"].get<double>(),
		j["stability_factor"].get<double>()
	);

	DragTable table;
	for (const auto& entry : j["drag_table"])
		table.push_back({ entry["velocity_ms"].get<double>(), entry["cd"].get<double>() });
	p.setDragTable(std::move(table));
	return p;
}

DragTable Application::loadDragTable()
{
	std::vector<dragCdTableEntry> newTable;
	// Example hardcoded drag coefficient table entries
	newTable.push_back(dragCdTableEntry{0.0,    0.17});
	newTable.push_back(dragCdTableEntry{100.0,  0.17});
	newTable.push_back(dragCdTableEntry{200.0,  0.18});
	newTable.push_back(dragCdTableEntry{300.0,  0.23});
	newTable.push_back(dragCdTableEntry{350.0,  0.36});
	newTable.push_back(dragCdTableEntry{400.0,  0.47});
	newTable.push_back(dragCdTableEntry{450.0,  0.43});
	newTable.push_back(dragCdTableEntry{500.0,  0.38});
	newTable.push_back(dragCdTableEntry{600.0,  0.33});
	newTable.push_back(dragCdTableEntry{700.0,  0.29});
	newTable.push_back(dragCdTableEntry{800.0,  0.26});
	newTable.push_back(dragCdTableEntry{900.0,  0.24});
	newTable.push_back(dragCdTableEntry{1000.0, 0.22});



	return newTable;
}
