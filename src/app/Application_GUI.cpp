#include "Application.hpp"

#include <algorithm>
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

	// ── Trajectory info + ballistic table ────────────────────────────────────
	if (!m_listOfTrajectories.empty()) {
		const Trajectory& lastTraj = m_listOfTrajectories.back();
		if (!lastTraj.empty()) {
			ImGui::Separator();
			ImGui::Text("Trajectory points : %d", (int)lastTraj.size());
			double dist = glm::distance(lastTraj.back().position, lastTraj.front().position);
			ImGui::Text("Impact distance   : %.1f m", dist);
		}

		if (ImGui::CollapsingHeader("Ballistic table")) {
			if (ImGui::Button("Clear All")) {
				m_listOfTrajectories.clear();
				m_listOfProjectiles.clear();
				m_trajectoryAzimuths.clear();
				m_trajectoryLabels.clear();
				m_renderer.clearTrajectories();
			}

			const double sampleInterval = 500.0;
			const double physDt         = 0.01;
			int removeIdx = -1;

			for (int ti = 0; ti < (int)m_listOfTrajectories.size(); ti++) {
				const Trajectory& t = m_listOfTrajectories[ti];
				if (t.empty()) continue;

				std::string header = (ti < (int)m_trajectoryLabels.size())
					? m_trajectoryLabels[ti]
					: ("Trajectory " + std::to_string(ti + 1));
				header += "##bt" + std::to_string(ti);

				if (ImGui::CollapsingHeader(header.c_str())) {
					double azDeg = (ti < (int)m_trajectoryAzimuths.size()) ? m_trajectoryAzimuths[ti] : 0.0;
					double azRad = glm::radians(azDeg);
					glm::dvec2 right = { std::sin(azRad), -std::cos(azRad) };

					std::string tableId = "balltable" + std::to_string(ti);
					if (ImGui::BeginTable(tableId.c_str(), 5,
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
						ImGui::EndTable();
					}
					static const char* separators[] = { "Comma (,)", "Semicolon (;)", "Tab (\\t)" };
					ImGui::SetNextItemWidth(130.0f);
					ImGui::Combo("##sep", &m_csvSeparator, separators, 3);
					ImGui::SameLine();
					if (ImGui::Button("Export CSV"))
						exportTrajectoryTableToCSV(t);
					ImGui::PushID(ti);
					ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.7f, 0.1f, 0.1f, 1.0f));
					float clearWidth = ImGui::CalcTextSize("Clear").x + ImGui::GetStyle().FramePadding.x * 2.0f;
					ImGui::SetCursorPosX(ImGui::GetContentRegionMax().x - clearWidth);
					if (ImGui::Button("Clear"))
						removeIdx = ti;
					ImGui::PopStyleColor();
					ImGui::PopID();
				}
			}

			if (removeIdx >= 0) {
				m_listOfTrajectories.erase(m_listOfTrajectories.begin() + removeIdx);
				m_listOfProjectiles.erase(m_listOfProjectiles.begin() + removeIdx);
				if (removeIdx < (int)m_trajectoryAzimuths.size())
					m_trajectoryAzimuths.erase(m_trajectoryAzimuths.begin() + removeIdx);
				if (removeIdx < (int)m_trajectoryLabels.size())
					m_trajectoryLabels.erase(m_trajectoryLabels.begin() + removeIdx);
				m_renderer.clearTrajectories();
				for (const auto& traj : m_listOfTrajectories)
					m_renderer.addTrajectory(traj);
			}
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

	updateLauncherManagerGUI();
}

void Application::updateLauncherManagerGUI()
{
	ImGuiIO& io = ImGui::GetIO();
	ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x - 10.0f, 10.0f), ImGuiCond_Always, ImVec2(1.0f, 0.0f));
	ImGui::SetNextWindowSizeConstraints(ImVec2(300.0f, 0.0f), ImVec2(400.0f, io.DisplaySize.y - 20.0f));
	ImGui::Begin("Launcher Manager", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove);

	// Keep parallel vectors in sync
	m_launcherProjectile.resize(m_launcher.size(), m_projectile);
	m_launcherSelected.resize(m_launcher.size(), false);
	m_solvedFireSolutions.resize(m_launcher.size());
	m_solvedFireSolutions.resize(m_launcher.size());

	if (ImGui::Button("+ Add Launcher")) {
		double groundZ = m_terrain->heightAt(0.0f, 0.0f);
		if (!m_launcherCatalog.empty()) {
			std::string entry = m_launcherCatalog[0];
			Launcher newL = loadLauncherFromJson(entry);
			newL.setPosition({0.0, 0.0, groundZ + 1.0});
			m_launcher.push_back(newL);
			Projectile newP = !m_compatibleProjectiles.empty()
				? loadProjectileFromJson(m_compatibleProjectiles[0].filename, m_launcher.back().getLatitudeInRad())
				: m_projectile;
			if (!m_compatibleProjectiles.empty())
				m_launcher.back().setSpeed(m_compatibleProjectiles[0].muzzleVelocity);
			m_launcherProjectile.push_back(newP);
			m_launcher.back().setLauncherType(entry);
		} else {
			m_launcher.push_back(Launcher({0.0, 0.0, groundZ + 1.0}, 0.0, 45.0, 900.0));
			m_launcherProjectile.push_back(m_launcherProjectile.empty() ? m_projectile : m_launcherProjectile.front());
		}
		m_launcherSelected.push_back(false);
		m_placementQueueIdx = 0;
	}

	ImGui::SameLine();
	ImGui::Checkbox("Instant trajectory", &m_instantFire);

	ImGui::Separator();

	// Launcher list — checkbox selects for editing and firing
	for (int i = 0; i < (int)m_launcher.size(); i++) {
		ImGui::PushID(i);

		bool sel = m_launcherSelected[i];
		if (ImGui::Checkbox("##sel", &sel)) {
			m_launcherSelected[i] = sel;
			m_placementQueueIdx = 0;
		}
		ImGui::SameLine();

		char label[32];
		snprintf(label, sizeof(label), "Launcher %d", i + 1);

		bool hasSolutions = i < (int)m_solvedFireSolutions.size() &&
			std::any_of(m_solvedFireSolutions[i].begin(), m_solvedFireSolutions[i].end(),
				[](const FireSolution& s) { return s.valid; });
		if (hasSolutions) {
			if (ImGui::TreeNode(label)) {
				const SolvedFireSolutions& sols = m_solvedFireSolutions[i];
				const char* attackLabels[] = { "Direct attack", "High angle" };
				for (int s = 0; s < (int)sols.size(); s++) {
					const FireSolution& sol = sols[s];
					const char* attackLabel = s < 2 ? attackLabels[s] : "Solution";
					ImGui::PushID(s);
					if (sol.valid) {
						ImGui::Text("%s: Az %.1f  El %.1f  TOF %.1fs", attackLabel, sol.azimuth_deg, sol.elevation_deg, sol.tof_s);
						ImGui::SameLine();
						if (ImGui::SmallButton("Apply")) {
							m_launcher[i].setAzimuth(sol.azimuth_deg);
							m_launcher[i].setElevation(sol.elevation_deg);
						}
					} else {
						ImGui::TextDisabled("%s: N/A", attackLabel);
					}
					ImGui::PopID();
				}
				ImGui::TreePop();
			}
		} else {
			ImGui::TextUnformatted(label);
		}

		ImGui::PopID();
	}

	// Collect selected indices
	int firstSelected = -1;
	int selectedCount = 0;
	for (int i = 0; i < (int)m_launcherSelected.size(); i++) {
		if (m_launcherSelected[i]) {
			if (firstSelected == -1) firstSelected = i;
			selectedCount++;
		}
	}

	if (selectedCount == 0) {
		ImGui::End();
		return;
	}

	ImGui::Separator();
	if (selectedCount == 1)
		ImGui::Text("Editing: Launcher %d", firstSelected + 1);
	else
		ImGui::Text("Editing: %d launchers", selectedCount);

	// ── Check if all selected are the same catalog type ────────────────────
	bool allSameType = true;
	const std::string& commonType = m_launcher[firstSelected].getLauncherType();
	for (int i = 0; i < (int)m_launcher.size(); i++) {
		if (m_launcherSelected[i] && m_launcher[i].getLauncherType() != commonType) {
			allSameType = false;
			break;
		}
	}

	// ── Launcher catalog dropdown ──────────────────────────────────────────
	if (!m_launcherCatalog.empty()) {
		const char* typeLabel = allSameType
			? (commonType.empty() ? "(custom)" : commonType.c_str())
			: "(mixed)";
		if (ImGui::BeginCombo("Type##lmgr", typeLabel)) {
			for (const auto& entry : m_launcherCatalog) {
				bool typeSel = allSameType && commonType == entry;
				if (ImGui::Selectable(entry.c_str(), typeSel)) {
					for (int i = 0; i < (int)m_launcher.size(); i++) {
						if (!m_launcherSelected[i]) continue;
						glm::dvec3 savedPos = m_launcher[i].getPosition();
						m_launcher[i] = loadLauncherFromJson(entry);
						m_launcher[i].setPosition(savedPos);
						m_launcher[i].setLauncherType(entry);
						if (!m_compatibleProjectiles.empty()) {
							m_launcherProjectile[i] = loadProjectileFromJson(m_compatibleProjectiles[0].filename, m_launcher[i].getLatitudeInRad());
							m_launcher[i].setSpeed(m_compatibleProjectiles[0].muzzleVelocity);
						}
					}
				}
				if (typeSel) ImGui::SetItemDefaultFocus();
			}
			ImGui::EndCombo();
		}
	}

	// ── Ballistic params (apply to all selected) ───────────────────────────
	Launcher& ref   = m_launcher[firstSelected];
	float azimuth   = (float)ref.getAzimuth();
	float elevation = (float)ref.getElevation();
	float speed     = (float)ref.getSpeed();
	float latitude  = (float)ref.getLatitude();

	bool changed = false;
	changed |= ImGui::DragFloat("Azimuth (deg)",         &azimuth,   0.5f, -180.0f, 180.0f);
	changed |= ImGui::DragFloat("Elevation (deg)",        &elevation, 0.5f,    0.0f,  90.0f);
	changed |= ImGui::DragFloat("Muzzle velocity (m/s)", &speed,     1.0f,    1.0f, 2000.0f);
	changed |= ImGui::DragFloat("Latitude (deg)",         &latitude,  0.5f,  -90.0f,  90.0f);
	if (changed) {
		for (int i = 0; i < (int)m_launcher.size(); i++) {
			if (!m_launcherSelected[i]) continue;
			m_launcher[i].setAzimuth((double)azimuth);
			m_launcher[i].setElevation((double)elevation);
			m_launcher[i].setSpeed((double)speed);
			m_launcher[i].setLatitude((double)latitude);
		}
	}

	// ── Projectile ────────────────────────────────────────────────────────
	ImGui::Separator();
	if (!allSameType) {
		ImGui::TextDisabled("Mixed launcher types — projectile editing unavailable");
	} else if (!m_compatibleProjectiles.empty()) {
		const std::string& projCurrent = m_compatibleProjectiles[m_selectedProjectile].filename;
		if (ImGui::BeginCombo("Projectile##lmgr", projCurrent.c_str())) {
			for (int i = 0; i < (int)m_compatibleProjectiles.size(); i++) {
				bool pSel = (m_selectedProjectile == i);
				if (ImGui::Selectable(m_compatibleProjectiles[i].filename.c_str(), pSel)) {
					m_selectedProjectile = i;
					for (int j = 0; j < (int)m_launcher.size(); j++) {
						if (!m_launcherSelected[j]) continue;
						m_launcherProjectile[j] = loadProjectileFromJson(m_compatibleProjectiles[i].filename, m_launcher[j].getLatitudeInRad());
						m_launcher[j].setSpeed(m_compatibleProjectiles[i].muzzleVelocity);
					}
				}
				if (pSel) ImGui::SetItemDefaultFocus();
			}
			ImGui::EndCombo();
		}
		Projectile& projRef = m_launcherProjectile[firstSelected];
		float mass     = (float)projRef.getMass();
		float diameter = (float)projRef.getDiameter();
		float twist    = (float)projRef.getTwistRate();
		float sf       = (float)projRef.getStabilityFactor();
		bool projChanged = false;
		projChanged |= ImGui::DragFloat("Mass (kg)",          &mass,     0.01f,  0.01f, 100.0f);
		projChanged |= ImGui::DragFloat("Diameter (m)",       &diameter, 0.001f, 0.001f,  0.5f);
		projChanged |= ImGui::DragFloat("Twist rate (rev/m)", &twist,    0.01f,  0.01f,  10.0f);
		projChanged |= ImGui::DragFloat("Stability factor",   &sf,       0.01f,  0.5f,    5.0f);
		if (projChanged) {
			for (int i = 0; i < (int)m_launcher.size(); i++) {
				if (!m_launcherSelected[i]) continue;
				m_launcherProjectile[i].setMass((double)mass);
				m_launcherProjectile[i].setDiameter((double)diameter);
				m_launcherProjectile[i].setTwistRate((double)twist);
				m_launcherProjectile[i].setStabilityFactor((double)sf);
			}
		}
	}

	// ── Placement / Remove ────────────────────────────────────────────────
	ImGui::Separator();
	if (selectedCount > 1)
		ImGui::TextDisabled("Press M to place selected launchers one by one");

	if (selectedCount == 1) {
		if (ImGui::Button("Place on map"))
			setLauncherToMap(m_launcher[firstSelected]);
	}

	if (selectedCount == 1 && (int)m_launcher.size() > 1) {
		ImGui::SameLine();
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.7f, 0.1f, 0.1f, 1.0f));
		if (ImGui::Button("Remove")) {
			m_launcher.erase(m_launcher.begin() + firstSelected);
			m_launcherProjectile.erase(m_launcherProjectile.begin() + firstSelected);
			m_launcherSelected.erase(m_launcherSelected.begin() + firstSelected);
			m_solvedFireSolutions.erase(m_solvedFireSolutions.begin() + firstSelected);
			m_placementQueueIdx = 0;
		}
		ImGui::PopStyleColor();
	}

	ImGui::End();
}
