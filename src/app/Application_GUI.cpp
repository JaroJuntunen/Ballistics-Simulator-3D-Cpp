#include "Application.hpp"

#include <imgui.h>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_opengl3.h>
#include <filesystem>
#include <cstring>

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

	// ── Terrain ───────────────────────────────────────────────────────────────
	if (ImGui::CollapsingHeader("Terrain")) {
		const char* proceduralLabel = "Procedural (Perlin)";
		const char* currentLabel = (m_selectedTerrain == -1)
			? proceduralLabel
			: std::filesystem::path(m_terrainCatalog[m_selectedTerrain]).stem().string().c_str();

		if (ImGui::BeginCombo("Tile##terrain", currentLabel)) {
			if (ImGui::Selectable(proceduralLabel, m_selectedTerrain == -1)) {
				m_selectedTerrain = -1;
				switchTerrain("");
			}
			if (m_selectedTerrain == -1) ImGui::SetItemDefaultFocus();

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

	// ── Wind ──────────────────────────────────────────────────────────────────
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

	// ── Launcher ──────────────────────────────────────────────────────────────
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
		ImGui::DragFloat("Azimuth (deg)",          &azimuth,   0.5f, -180.0f, 180.0f);
		ImGui::DragFloat("Elevation (deg)",         &elevation, 0.5f,    0.0f,  90.0f);
		ImGui::DragFloat("Muzzle velocity (m/s)",  &speed,     1.0f,    1.0f, 2000.0f);
		ImGui::DragFloat("Latitude (deg)",          &latitude,  0.5f,  -90.0f,  90.0f);
		m_launcher.setAzimuth((double)azimuth);
		m_launcher.setElevation((double)elevation);
		m_launcher.setSpeed((double)speed);
		m_launcher.setLatitude((double)latitude);
		ImGui::Checkbox("Instant trajectory", &m_instantFire);
	}

	// ── Projectile ────────────────────────────────────────────────────────────
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

	// ── Trajectory info + ballistic table ────────────────────────────────────
	if (!m_listOfTrajectories.empty()) {
		const Trajectory& traj = m_listOfTrajectories.back();
		if (!traj.empty()) {
			ImGui::Separator();
			ImGui::Text("Trajectory points : %d", (int)traj.size());
			double dist = glm::distance(traj.back().position, traj.front().position);
			ImGui::Text("Impact distance   : %.1f m", dist);
		}

		if (ImGui::CollapsingHeader("Ballistic table")) {
			const Trajectory& t         = m_listOfTrajectories.back();
			const double sampleInterval = 500.0;
			const double physDt         = 0.01;

			double azRad = glm::radians(m_launcher.getAzimuth());
			glm::dvec2 right = { std::sin(azRad), -std::cos(azRad) };

			if (ImGui::BeginTable("balltable", 5,
				ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY,
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
				exportTrajectoryTableToCSV(t);
		}
	}

	// ── Scenario ──────────────────────────────────────────────────────────────
	if (ImGui::CollapsingHeader("Scenario")) {
		static char scenarioName[128] = "scenario1";
		ImGui::SetNextItemWidth(200.0f);
		ImGui::InputText("Name##scenario", scenarioName, sizeof(scenarioName));

		if (ImGui::Button("Save")) {
			std::string path = "data/scenarios/";
			path += scenarioName;
			path += ".json";
			saveScenario(path);
		}
		ImGui::SameLine();
		if (ImGui::Button("Load")) {
			std::string path = "data/scenarios/";
			path += scenarioName;
			path += ".json";
			loadScenario(path);
		}

		ImGui::Spacing();
		const std::string scenarioDir = "data/scenarios";
		if (std::filesystem::exists(scenarioDir)) {
			for (const auto& entry : std::filesystem::directory_iterator(scenarioDir)) {
				if (entry.path().extension() == ".json") {
					std::string stem = entry.path().stem().string();
					if (ImGui::Selectable(stem.c_str()))
						std::strncpy(scenarioName, stem.c_str(), sizeof(scenarioName) - 1);
				}
			}
		}
	}

	ImGui::End();
}
