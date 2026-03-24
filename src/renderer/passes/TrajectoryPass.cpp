#include "TrajectoryPass.hpp"
#include <iostream>

void TrajectoryPass::init()
{
	if (!m_shader.load("data/shaders/basic.vert", "data/shaders/basic.frag")) {
		std::cerr << "TrajectoryPass: failed to load shaders\n";
		return;
	}
}

void TrajectoryPass::upload(const std::vector<RigidBodyState> &points)
{
	if (m_vao) {cleanup();}
	std::vector<float> vertices;
	m_vertexCount = static_cast<int>(points.size());
	vertices.reserve(m_vertexCount * 3);

	for (const auto& state : points) {
		vertices.push_back(static_cast<float>(state.position.x));
		vertices.push_back(static_cast<float>(state.position.y));
		vertices.push_back(static_cast<float>(state.position.z));
	}

	glGenVertexArrays(1, &m_vao);
	glGenBuffers(1, &m_vbo);

	glBindVertexArray(m_vao);
	glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
	glBufferData(GL_ARRAY_BUFFER,
				 vertices.size() * sizeof(float),
				 vertices.data(),
				 GL_DYNAMIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), nullptr);
	glEnableVertexAttribArray(0);

	glBindVertexArray(0);
}


void TrajectoryPass::render(const glm::mat4 &view, const glm::mat4 &proj)
{
	if (!m_vao) return;

	m_shader.bind();
	m_shader.setMat4("uMVP",   proj * view);
	m_shader.setVec3("uColor", glm::vec3(1.0f,1.0f,1.0f	)); // white

	glBindVertexArray(m_vao);
	glDrawArrays(GL_LINE_STRIP, 0, m_vertexCount);
	glBindVertexArray(0);

	m_shader.unbind();
}

void TrajectoryPass::cleanup()
{
	
	if (m_vao) { glDeleteVertexArrays(1, &m_vao); m_vao = 0; }
	if (m_vbo) { glDeleteBuffers(1, &m_vbo);      m_vbo = 0; }
}
