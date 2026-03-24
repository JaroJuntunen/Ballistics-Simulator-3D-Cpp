#pragma once

#include "core/Camera.hpp"
#include "passes/TerrainPass.hpp"
#include "simulation/environment/Terrain.hpp"

// Drives the OpenGL frame. Owns all render passes (added per phase).
// beginFrame: clear buffers, set viewport, store view/proj matrices.
// endFrame:   flush ImGui draw data.
class Renderer {
public:
	void initTerrain(const Terrain& terrain, int samples = 100);

	void beginFrame(int width, int height, const Camera& camera);
	void endFrame();

	glm::mat4 view() const { return m_view; }
	glm::mat4 proj() const { return m_proj; }

private:
	TerrainPass m_terrainPass;

	glm::mat4 m_view{1.0f};
	glm::mat4 m_proj{1.0f};
};
