#include "Application.hpp"

#include <SDL3/SDL.h>
#include <imgui.h>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_opengl3.h>
#include <iostream>

bool Application::init() {
	if (!m_context.init("Ballistics Simulator 3D", 1600, 900))
		return false;

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui::StyleColorsDark();
	ImGui_ImplSDL3_InitForOpenGL(m_context.window(), m_context.glContext());
	ImGui_ImplOpenGL3_Init("#version 330 core");

	m_camera.setOrbit(45.0f, 30.0f, 1800.0f);
	m_renderer.initTerrain(m_terrain, m_terrain.extent() / 100);
	m_renderer.initTrajectory();
	double groundZ = m_terrain.heightAt(0.0f, 0.0f);
	m_launcher = Launcher({0.0, 0.0, groundZ + 1.0}, 0.0, 45.0, 900.0);
	m_projectile = Projectile(48, 0.155, 0.3, 0.322580645, 1.2);
	m_projectile.setDragTable(loadDragTable());
	m_running = true;
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
		
		handleInput();

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplSDL3_NewFrame();
		ImGui::NewFrame();

		ImGui::Begin("Ballistics Simulator 3D");
		ImGui::Text("Phase 1 — Foundation");
		ImGui::Separator();
		ImGui::Text("Left drag : orbit");
		ImGui::Text("Right drag: pan");
		ImGui::Text("Scroll    : zoom");
		ImGui::End();

		render();
	}

	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplSDL3_Shutdown();
	ImGui::DestroyContext();
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
		RigidBodyState initialState = m_launcher.fire(m_projectile);
		StopFn stop = [&](const RigidBodyState& s) {
			return BallisticsModel::hasImpacted(s, m_terrain);
		};
		m_trajectory = Integrator::simulateSteps(initialState, m_projectile, m_wind, m_launcher.getLatitudeInRad(), 0.01, BallisticsModel::derivative, stop);
		std::cout << "projectile trejectory length : " << glm::distance(m_trajectory[m_trajectory.size() - 1].position, m_trajectory[0].position) << "\n";
		m_renderer.uploadTrajectory(m_trajectory);
	}


}

void Application::render() {
	int w, h;
	SDL_GetWindowSizeInPixels(m_context.window(), &w, &h);

	m_renderer.beginFrame(w, h, m_camera);
	m_renderer.endFrame();

	m_context.swapBuffers();
}

std::shared_ptr<std::vector<dragCdTableEntry>> Application::loadDragTable()
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



	return std::make_shared<std::vector<dragCdTableEntry>>(newTable);
}
