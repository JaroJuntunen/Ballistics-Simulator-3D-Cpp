#pragma once

#include <vector>
#include <glm/glm.hpp>
#include <glad/gl.h>
#include <glm/gtc/matrix_transform.hpp>
#include "simulation/core/RigidBodyState.hpp"
#include "renderer/core/ShaderProgram.hpp"
	
class TrajectoryPass {
public:
	void init();
	void upload(const std::vector<RigidBodyState>& points);
	void render(const glm::mat4& view, const glm::mat4& proj);
	void cleanup();
private:
	ShaderProgram	m_shader;
	unsigned int	m_vao        = 0;
	unsigned int	m_vbo        = 0;
	int				m_vertexCount = 0;
};
