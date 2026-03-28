#include "Application.hpp"

#include <SDL3/SDL.h>
#include <imgui.h>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_opengl3.h>
#include <iostream>

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
	m_launcher.push_back(Launcher({0.0,0.0,0.0}, 0.0, 45.0, 100.0));
	Launcher& l = m_launcher.front();
	if (!m_launcherCatalog.empty()) {
		l = loadLauncherFromJson(m_launcherCatalog[0]);
		l.setPosition({0.0, 0.0, groundZ + 1.0});
		if (!m_compatibleProjectiles.empty()) {
			m_projectile = loadProjectileFromJson(m_compatibleProjectiles[0].filename);
			l.setSpeed(m_compatibleProjectiles[0].muzzleVelocity);
		}
		l.setLauncherType(m_launcherCatalog[0]);
	} else {
		l = Launcher({0.0, 0.0, groundZ + 1.0f}, 0.0, 45.0, 900.0);
		m_projectile = Projectile(48, 0.155, 0.3, 0.322580645, 1.2, l.getLatitudeInRad());
		m_projectile.setDragTable(loadDragTable());
	}
	m_launcherProjectile.push_back(m_projectile);
	m_launcherSelected.push_back(false);
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
	for (int i = 0; i < (int)m_listOfProjectiles.size(); i++) {
		Projectile& proj = m_listOfProjectiles.at(i);
		Trajectory& traj = m_listOfTrajectories.at(i);

		if (!proj.getIsImpacted()) {
			traj.push_back(Integrator::step(traj.back(), proj, m_wind, dt, BallisticsModel::derivative));
			proj.setIsImpacted(BallisticsModel::hasImpacted(traj.back(), *m_terrain));
			m_renderer.updateTrajectory(i, traj);
		}
	}
}

glm::dvec3 Application::rayFromMouse(float screenX, float screenY)
{
	int w, h;
	SDL_GetWindowSizeInPixels(m_context.window(), &w, &h);

	float ndcX = (2.0f * screenX / w) - 1.0f;
	float ndcY = 1.0f - (2.0f * screenY / h);

	glm::mat4 invVP = glm::inverse(m_renderer.proj() * m_renderer.view());
	glm::vec4 nearPt = invVP * glm::vec4(ndcX, ndcY, -1.0f, 1.0f);
	glm::vec4 farPt  = invVP * glm::vec4(ndcX, ndcY,  1.0f, 1.0f);

	nearPt /= nearPt.w;
	farPt  /= farPt.w;

	return glm::normalize(glm::dvec3(farPt - nearPt));
}

void Application::setLauncherToMap(Launcher& l)
{
	glm::dvec3 rayOrigin = m_camera.position();
	glm::dvec3 rayDir    = rayFromMouse(m_input.state().mousePosX, m_input.state().mousePosY);

	for (double t = 0; t < 200000.0; t += 10.0) {
		glm::dvec3 pt = rayOrigin + rayDir * t;
		if (pt.z <= m_terrain->heightAt((float)pt.x, (float)pt.y)) {
			pt.z = m_terrain->heightAt((float)pt.x, (float)pt.y) + 1.0;
			l.setPosition(pt);
			break;
		}
	}
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
	auto* srtm = dynamic_cast<SRTMTerrain*>(m_terrain.get());
	for (Launcher& l : m_launcher) {
		glm::dvec3 pos = l.getPosition();
		pos.z = m_terrain->heightAt((float)pos.x, (float)pos.y) + 1.0;
		l.setPosition(pos);
		if (srtm) {
			double latFraction = pos.y / srtm->height();
			l.setLatitude(srtm->originLat() + latFraction);
		}
	}
}

void Application::handleInput() {
	const InputState& in = m_input.state();
	const ImGuiIO&    io = ImGui::GetIO();

	if (io.WantCaptureMouse)
		return;

	if (in.mouseLeft)
		m_camera.orbit(-in.mouseDeltaX * 0.4f, in.mouseDeltaY * 0.4f);

	if (in.mouseRight)
		m_camera.pan(in.mouseDeltaX, in.mouseDeltaY);

	if (in.scrollY != 0.0f)
		m_camera.zoom(in.scrollY * -40.0f);

	if (in.space) {
		StopFn stop = [&](const RigidBodyState& s) {
			return BallisticsModel::hasImpacted(s, *m_terrain);
		};
		for (int i = 0; i < (int)m_launcher.size(); i++) {
			if (!m_launcherSelected[i]) continue;
			Projectile proj = m_launcherProjectile[i];
			RigidBodyState initialState = m_launcher[i].fire(proj);
			m_listOfProjectiles.push_back(proj);
			if (m_instantFire)
				m_trajectory = Integrator::simulateSteps(initialState, proj, m_wind, 0.01, BallisticsModel::derivative, stop);
			else {
				m_trajectory.clear();
				m_trajectory.push_back(initialState);
			}
			m_listOfTrajectories.push_back(m_trajectory);
			m_renderer.addTrajectory(m_trajectory);
			m_trajectoryAzimuths.push_back(m_launcher[i].getAzimuth());
			std::string label = "L" + std::to_string(i + 1);
			if (!m_launcher[i].getLauncherType().empty())
				label += " (" + m_launcher[i].getLauncherType() + ")";
			m_trajectoryLabels.push_back(label);
		}
	}

	if (in.keyM) {
		std::vector<int> selIndices;
		for (int i = 0; i < (int)m_launcherSelected.size(); i++)
			if (m_launcherSelected[i]) selIndices.push_back(i);
		if (selIndices.empty()) {
			setLauncherToMap(m_launcher[0]);
		} else {
			int idx = m_placementQueueIdx % (int)selIndices.size();
			setLauncherToMap(m_launcher[selIndices[idx]]);
			m_placementQueueIdx = (m_placementQueueIdx + 1) % (int)selIndices.size();
		}
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
