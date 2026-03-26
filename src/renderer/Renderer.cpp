#include "Renderer.hpp"

#include <glad/gl.h>
#include <imgui.h>
#include <imgui_impl_opengl3.h>

void Renderer::initTerrain(const Terrain& terrain, int samples) {
	m_terrainPass.init(terrain, samples);
}

void Renderer::initTrajectory(){
	m_trajectoryPass.init();
}

int Renderer::addTrajectory(const std::vector<RigidBodyState>& points) {
	return m_trajectoryPass.add(points);
}

void Renderer::updateTrajectory(int index, const std::vector<RigidBodyState>& points) {
	m_trajectoryPass.update(index, points);
}

void Renderer::clearTrajectories() {
	m_trajectoryPass.clearAll();
}

void Renderer::beginFrame(int width, int height, const Camera &camera)
{
	glViewport(0, 0, width, height);
	glClearColor(0.08f, 0.10f, 0.12f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);

	float aspect = static_cast<float>(width) / static_cast<float>(height);
	m_view = camera.viewMatrix();
	m_proj = camera.projectionMatrix(aspect);

	m_terrainPass.render(m_view, m_proj);
	m_trajectoryPass.render(m_view, m_proj);
}

void Renderer::endFrame() {
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}
