#pragma once

#include <glm/glm.hpp>

class Wind {
	public:
		Wind();
		~Wind() = default;
		glm::dvec3 getWindSpeed();
		void	passWindTime(double dt) {m_time += dt;};
	private:
	glm::dvec3	m_baseWindSpeed;
	double		m_windGustSeverity;
	double		m_gustFrequency;

	double m_time;
};