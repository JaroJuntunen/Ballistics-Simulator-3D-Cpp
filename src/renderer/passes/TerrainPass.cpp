#include "TerrainPass.hpp"

#include <glad/gl.h>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>
#include <iostream>

void TerrainPass::init(const Terrain& terrain, int samples) {
	if (!m_shader.load("data/shaders/basic.vert", "data/shaders/basic.frag")) {
		std::cerr << "TerrainPass: failed to load shaders\n";
		return;
	}

	const float half  = terrain.extent() * 0.5f;
	const float step  = terrain.extent() / static_cast<float>(samples - 1);

	// ── Vertices ─────────────────────────────────────────────────────────────
	// Each vertex is (x, y, z) — z comes from heightAt
	std::vector<float> vertices;
	vertices.reserve(samples * samples * 3);

	for (int row = 0; row < samples; ++row) {
		for (int col = 0; col < samples; ++col) {
			float x = -half + col * step;
			float y = -half + row * step;
			float z = terrain.heightAt(x, y);
			vertices.push_back(x);
			vertices.push_back(y);
			vertices.push_back(z);
		}
	}

	// ── Indices ───────────────────────────────────────────────────────────────
	// Two triangles per quad, wound counter-clockwise
	std::vector<unsigned int> indices;
	indices.reserve((samples - 1) * (samples - 1) * 6);

	for (int row = 0; row < samples - 1; ++row) {
		for (int col = 0; col < samples - 1; ++col) {
			unsigned int tl = row * samples + col;
			unsigned int tr = tl + 1;
			unsigned int bl = tl + samples;
			unsigned int br = bl + 1;

			indices.push_back(tl);
			indices.push_back(bl);
			indices.push_back(tr);

			indices.push_back(tr);
			indices.push_back(bl);
			indices.push_back(br);
		}
	}

	m_indexCount = static_cast<int>(indices.size());

	// ── Upload to GPU ─────────────────────────────────────────────────────────
	glGenVertexArrays(1, &m_vao);
	glGenBuffers(1, &m_vbo);
	glGenBuffers(1, &m_ebo);

	glBindVertexArray(m_vao);

	glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
	glBufferData(GL_ARRAY_BUFFER,
				 vertices.size() * sizeof(float),
				 vertices.data(),
				 GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER,
				 indices.size() * sizeof(unsigned int),
				 indices.data(),
				 GL_STATIC_DRAW);

	// location 0: vec3 position
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), nullptr);
	glEnableVertexAttribArray(0);

	glBindVertexArray(0);
}

void TerrainPass::render(const glm::mat4& view, const glm::mat4& proj) {
	if (!m_vao) return;

	m_shader.bind();
	m_shader.setMat4("uMVP",   proj * view);
	m_shader.setVec3("uColor", glm::vec3(0.35f, 0.50f, 0.30f)); // muted green

	glBindVertexArray(m_vao);
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); // wireframe so the mesh shape is visible
	glDrawElements(GL_TRIANGLES, m_indexCount, GL_UNSIGNED_INT, nullptr);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glBindVertexArray(0);

	m_shader.unbind();
}

void TerrainPass::cleanup() {
	if (m_vao) { glDeleteVertexArrays(1, &m_vao); m_vao = 0; }
	if (m_vbo) { glDeleteBuffers(1, &m_vbo);      m_vbo = 0; }
	if (m_ebo) { glDeleteBuffers(1, &m_ebo);      m_ebo = 0; }
	m_indexCount = 0;
}
