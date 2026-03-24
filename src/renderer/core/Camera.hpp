#pragma once

#include <glm/glm.hpp>

// Orbit camera — rotates around a target point using spherical coordinates.
// theta: horizontal angle (degrees), phi: vertical angle (degrees, clamped ±89°)
// radius: distance from target
class Camera {
public:
	void setOrbit(float theta, float phi, float radius);

	void orbit(float dTheta, float dPhi);   // rotate around target
	void pan(float dx, float dy);           // translate target in camera-local plane
	void zoom(float delta);                 // move closer/farther from target

	glm::mat4 viewMatrix()                  const;
	glm::mat4 projectionMatrix(float aspect) const;
	glm::vec3 position()                    const;

private:
	float     m_theta  = 45.0f;
	float     m_phi    = 30.0f;
	float     m_radius = 500.0f;
	glm::vec3 m_target = {0.0f, 0.0f, 0.0f};

	float m_fovDeg = 60.0f;
	float m_near   = 1.0f;
	float m_far    = 200000.0f;
};
