#include "Camera.hpp"

#include <glm/gtc/matrix_transform.hpp>
#include <algorithm>
#include <cmath>

static constexpr float DEG2RAD = 3.14159265f / 180.0f;

void Camera::setOrbit(float theta, float phi, float radius) {
	m_theta  = theta;
	m_phi    = phi;
	m_radius = radius;
}

void Camera::orbit(float dTheta, float dPhi) {
	m_theta += dTheta;
	m_phi    = std::clamp(m_phi + dPhi, -89.0f, 89.0f);
}

void Camera::pan(float dx, float dy) {
	glm::vec3 forward = glm::normalize(m_target - position());
	glm::vec3 right   = glm::normalize(glm::cross(forward, glm::vec3(0.0f, 0.0f, 1.0f)));
	glm::vec3 up      = glm::cross(right, forward);

	float scale = m_radius * 0.0008f;
	m_target -= right * dx * scale;
	m_target += up    * dy * scale;
}

void Camera::zoom(float delta) {
	m_radius = std::max(10.0f, m_radius + delta);
}

glm::vec3 Camera::position() const {
	float phiR   = m_phi   * DEG2RAD;
	float thetaR = m_theta * DEG2RAD;
	return m_target + glm::vec3(
		m_radius * std::cos(phiR) * std::cos(thetaR),
		m_radius * std::cos(phiR) * std::sin(thetaR),
		m_radius * std::sin(phiR)
	);
}

glm::mat4 Camera::viewMatrix() const {
	return glm::lookAt(position(), m_target, glm::vec3(0.0f, 0.0f, 1.0f));
}

glm::mat4 Camera::projectionMatrix(float aspect) const {
	return glm::perspective(glm::radians(m_fovDeg), aspect, m_near, m_far);
}
