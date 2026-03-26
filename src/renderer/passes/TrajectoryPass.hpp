#pragma once

#include <vector>
#include <glm/glm.hpp>
#include <glad/gl.h>
#include <glm/gtc/matrix_transform.hpp>
#include "simulation/core/RigidBodyState.hpp"
#include "renderer/core/ShaderProgram.hpp"

struct TrajectoryBuffer {
	unsigned int vao         = 0;
	unsigned int vbo         = 0;
	int          vertexCount = 0;
};

class TrajectoryPass {
public:
	void init();
	int  add(const std::vector<RigidBodyState>& points);           // returns index
	void update(int index, const std::vector<RigidBodyState>& points);
	void render(const glm::mat4& view, const glm::mat4& proj);
	void clearAll();
	int  count() const { return static_cast<int>(m_trajectories.size()); }

private:
	TrajectoryBuffer buildBuffer(const std::vector<RigidBodyState>& points);

	ShaderProgram                 m_shader;
	std::vector<TrajectoryBuffer> m_trajectories;

	static constexpr glm::vec3 k_colors[] = {
		{1.0f, 1.0f, 1.0f},   // white
		{1.0f, 1.0f, 0.0f},   // yellow
		{0.0f, 1.0f, 1.0f},   // cyan
		{1.0f, 0.4f, 0.0f},   // orange
		{0.5f, 1.0f, 0.0f},   // lime
		{1.0f, 0.0f, 1.0f},   // magenta
	};
	static constexpr int k_colorCount = 6;
};
