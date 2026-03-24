#pragma once

#include "simulation/environment/Terrain.hpp"
#include "renderer/core/ShaderProgram.hpp"
#include <glm/glm.hpp>

// Builds a grid mesh from a Terrain's heightAt() samples and renders it.
// The mesh is built once on init() and never rebuilt (Phase 1).
class TerrainPass {
public:
	// samples: number of vertices along each axis (e.g. 100 → 100×100 grid)
	void init(const Terrain& terrain, int samples = 100);
	void render(const glm::mat4& view, const glm::mat4& proj);
	void cleanup();

private:
	ShaderProgram m_shader;
	unsigned int  m_vao        = 0;
	unsigned int  m_vbo        = 0;
	unsigned int  m_ebo        = 0;
	int           m_indexCount = 0;
};
