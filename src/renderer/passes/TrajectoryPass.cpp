#include "TrajectoryPass.hpp"
#include <iostream>

void TrajectoryPass::init()
{
	if (!m_shader.load("data/shaders/basic.vert", "data/shaders/basic.frag"))
		std::cerr << "TrajectoryPass: failed to load shaders\n";
}

TrajectoryBuffer TrajectoryPass::buildBuffer(const std::vector<RigidBodyState>& points)
{
	TrajectoryBuffer buf;
	buf.vertexCount = static_cast<int>(points.size());

	std::vector<float> vertices;
	vertices.reserve(buf.vertexCount * 3);
	for (const auto& s : points) {
		vertices.push_back(static_cast<float>(s.position.x));
		vertices.push_back(static_cast<float>(s.position.y));
		vertices.push_back(static_cast<float>(s.position.z));
	}

	glGenVertexArrays(1, &buf.vao);
	glGenBuffers(1, &buf.vbo);
	glBindVertexArray(buf.vao);
	glBindBuffer(GL_ARRAY_BUFFER, buf.vbo);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_DYNAMIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), nullptr);
	glEnableVertexAttribArray(0);
	glBindVertexArray(0);

	return buf;
}

int TrajectoryPass::add(const std::vector<RigidBodyState>& points)
{
	m_trajectories.push_back(buildBuffer(points));
	return static_cast<int>(m_trajectories.size()) - 1;
}

void TrajectoryPass::update(int index, const std::vector<RigidBodyState>& points)
{
	if (index < 0 || index >= static_cast<int>(m_trajectories.size())) return;

	TrajectoryBuffer& buf = m_trajectories[index];
	if (buf.vao) {
		glDeleteVertexArrays(1, &buf.vao);
		glDeleteBuffers(1, &buf.vbo);
	}
	buf = buildBuffer(points);
}

void TrajectoryPass::render(const glm::mat4& view, const glm::mat4& proj)
{
	if (m_trajectories.empty()) return;

	m_shader.bind();
	m_shader.setMat4("uMVP", proj * view);

	for (int i = 0; i < static_cast<int>(m_trajectories.size()); ++i) {
		const TrajectoryBuffer& buf = m_trajectories[i];
		if (!buf.vao) continue;
		m_shader.setVec3("uColor", k_colors[i % k_colorCount]);
		glBindVertexArray(buf.vao);
		glDrawArrays(GL_LINE_STRIP, 0, buf.vertexCount);
		glBindVertexArray(0);
	}

	m_shader.unbind();
}

void TrajectoryPass::clearAll()
{
	for (auto& buf : m_trajectories) {
		if (buf.vao) glDeleteVertexArrays(1, &buf.vao);
		if (buf.vbo) glDeleteBuffers(1, &buf.vbo);
	}
	m_trajectories.clear();
}
