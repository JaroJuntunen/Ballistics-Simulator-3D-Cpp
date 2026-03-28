#include "Launcher.hpp"

RigidBodyState Launcher::fire(Projectile projectile) const
{
	RigidBodyState initialState;
	initialState.position = m_position;
	initialState.velocity = {
		m_speed * cos(glm::radians(m_elevation)) * cos(glm::radians(m_azimuth)),
		m_speed * cos(glm::radians(m_elevation)) * sin(glm::radians(m_azimuth)),
		m_speed * sin(glm::radians(m_elevation))
	};
	return initialState;
}
