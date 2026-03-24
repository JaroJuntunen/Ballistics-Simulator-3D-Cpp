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
	m_renderer.initTerrain(m_terrain);
	m_renderer.initTrajectory();
	double groundZ = m_terrain.heightAt(0.0f, 0.0f);
	m_launcher = Launcher({0.0, 0.0, groundZ + 1.0}, 0.0, 30.0, 800.0);

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
		std::cout << "projectile trejectory length : " << m_trajectory.size() << "\n";
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
